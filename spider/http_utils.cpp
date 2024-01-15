#include "http_utils.h"

#include <regex>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>

#include "../indexer/indexer.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = boost::asio::ip;
namespace ssl = boost::asio::ssl;

using tcp = boost::asio::ip::tcp;

bool isText(const boost::beast::multi_buffer::const_buffers_type& b)
{
	for (auto itr = b.begin(); itr != b.end(); itr++)
	{
		for (int i = 0; i < (*itr).size(); i++)
		{
			if (*((const char*)(*itr).data() + i) == 0)
			{
				return false;
			}
		}
	}

	return true;
}

std::string getHtmlContent(const Link& link)
{

	std::string result;

	try
	{
		std::string host = link.hostName;
		std::string query = link.query;

		net::io_context ioc;

		if (link.protocol == ProtocolType::HTTPS)
		{

			ssl::context ctx(ssl::context::tlsv13_client);
			ctx.set_default_verify_paths();

			beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
			stream.set_verify_mode(ssl::verify_none);

			stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
				return true; // Accept any certificate
				});


			if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
				beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
				throw beast::system_error{ec};
			}

			ip::tcp::resolver resolver(ioc);
			get_lowest_layer(stream).connect(resolver.resolve({ host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));


			http::request<http::empty_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			stream.handshake(ssl::stream_base::client);
			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec == net::error::eof) {
				ec = {};
			}

			if (ec) {
				throw beast::system_error{ec};
			}
		}
		else
		{
			tcp::resolver resolver(ioc);
			beast::tcp_stream stream(ioc);

			auto const results = resolver.resolve(host, "http");

			stream.connect(results);

			http::request<http::string_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);


			http::write(stream, req);

			beast::flat_buffer buffer;

			http::response<http::dynamic_body> res;


			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected)
				throw beast::system_error{ec};

		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return result;
}

std::string remover(const std::string& html)
{
	std::regex htmlTags("<.*?>");
	return std::regex_replace(html, htmlTags, "");
}

std::map<std::string, int> counter(const std::vector<std::string>& words)
{
	std::map<std::string, int> wordNum;

	for (int i = 0; i < words.size(); ++i)
	{
		auto word = toLowerCase(words[i]);
		if (wordNum.count(word) == 0)
		{
			wordNum[word] = 1;
		}
		else
		{
			wordNum[word]++;
		}
		
	}

	return wordNum;
}

std::vector<Link> getLinks(const std::string& html, ProtocolType protocol, const std::string& hostName)
{
	std::vector<std::string> links;
	std::vector<Link> result;

	//std::regex htmlTeg("<a href=\"(.*?)\"");
	std::regex htmlTeg("<a href=\"(.*?)(?=[\"|?|#])");

	auto words_begin = std::sregex_iterator(html.begin(), html.end(), htmlTeg);
	auto words_end = std::sregex_iterator();

	for (std::sregex_iterator i = words_begin; i != words_end; ++i)
	{
		std::smatch match = *i;
		std::string match_str = match.str();
		links.push_back(match_str.substr(9, match_str.size() - 9));

	}

	for (const auto& link : links)
	{
		if (link[0] == '/')
		{
			result.push_back({ ProtocolType::HTTPS, hostName, link });
		}
		else if ((link.substr(0, 7) == "http://") || (link.substr(0, 8) == "https://"))
		{
			//Link relust_temp;
			ProtocolType protocolTemp;
			std::string queryTemp;
			std::string hostNameTemp;

			std::string removeTag;

			if (link.substr(0, 7) == "http://")
			{
				removeTag = link.substr(7);
				protocolTemp = ProtocolType::HTTP;
			}
			else if (link.substr(0, 8) == "https://")
			{
				removeTag = link.substr(8);
				protocolTemp = ProtocolType::HTTPS;
			}

			size_t slash = removeTag.find('/');
			hostNameTemp = removeTag.substr(0, slash);

			if (slash == std::string::npos)
			{
				queryTemp = "/";
			}
			else
			{
				queryTemp = removeTag.substr(slash);
			}

			result.push_back({ protocolTemp, hostNameTemp, queryTemp });
		}

	}

	return result;

}

Link startLinks(const std::string& link)
{
	Link result;

	if (link.substr(0, 7) == "http://")
	{
		std::string withoutProtocol = link.substr(7);
		size_t slashPos = withoutProtocol.find('/');
		result.hostName = withoutProtocol.substr(0, slashPos);
		if (slashPos == std::string::npos)
		{
			result.query = "/";
		}
		else
		{
			result.query = withoutProtocol.substr(slashPos);
		}

		result.protocol = ProtocolType::HTTP;
	}
	else if (link.substr(0, 8) == "https://")
	{
		std::string withoutProtocol = link.substr(8);
		size_t slashPos = withoutProtocol.find('/');
		result.hostName = withoutProtocol.substr(0, slashPos);
		if (slashPos == std::string::npos)
		{
			result.query = "/";
		}
		else
		{
			result.query = withoutProtocol.substr(slashPos);
		}
		result.protocol = ProtocolType::HTTPS;
	}

	return result;
}