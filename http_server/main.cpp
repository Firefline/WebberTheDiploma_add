#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <string>
#include <iostream>

#include "http_connection.h"
#include <Windows.h>

#include "server.h"
#include "../config/config.h"
#include "../indexer/indexer.h"
#include "../database/database.h"

void httpServer(tcp::acceptor& acceptor, tcp::socket& socket, Server& server)
{
	acceptor.async_accept(socket,
		[&](beast::error_code ec)
		{
			if (!ec)
				std::make_shared<HttpConnection>(std::move(socket), server)->start();
			httpServer(acceptor, socket, server);
		});
}


int main(int argc, char* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try
	{
		Config config("C:/Users/Firef/source/repos/Webber/config/config.ini");
		Server server(config);

		auto const address = net::ip::make_address("0.0.0.0");
		//unsigned short port = 8080;
		unsigned short port = atoi(config.getConfig("http_server_port").c_str());

		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, { address, port }};
		tcp::socket socket{ioc};
		httpServer(acceptor, socket, server);

		system("start http://localhost:8080");

		ioc.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}