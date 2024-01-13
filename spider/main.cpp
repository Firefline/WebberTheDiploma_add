#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <functional>

#include "http_utils.h"
#include "../config/config.h"
#include "../indexer/indexer.h"
#include "../database/database.h"
#include "client.h"


std::mutex mtx;
std::condition_variable cv;
std::queue<std::function<void()>> tasks;
bool exitThreadPool = false;


void threadPoolWorker() {
	std::unique_lock<std::mutex> lock(mtx);
	while (!exitThreadPool || !tasks.empty()) {
		if (tasks.empty()) {
			cv.wait(lock);
		}
		else {
			auto task = tasks.front();
			tasks.pop();
			lock.unlock();
			task();
			lock.lock();
		}
	}
}
void parseLink(Link link, Client& client, int depth, int index, int count)
{
	try {

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::string html = getHtmlContent(link);

		if (html.size() == 0)
		{
			std::cout << "Failed to get HTML Content" << std::endl;
			return;
		}

		// TODO: Parse HTML code here on your own
		std::vector<Link> links = getLinks(html, link.protocol, link.hostName);

		for (auto a : links)
		{
			std::cout << a.query << std::endl;
		}
		std::string text = remover(html);
		std::vector<std::string> words = indexer(text);
		std::map<std::string, int> wordsTotal = counter(words);
		client.wordsDoc_new(link, wordsTotal);

		// TODO: Collect more links from HTML code and add them to the parser like that:

		/*std::vector<Link> links = {
			{ProtocolType::HTTPS, "en.wikipedia.org", "/wiki/Wikipedia"},
			{ProtocolType::HTTPS, "wikimediafoundation.org", "/"},
		};*/

		if (depth > 0) {
			std::lock_guard<std::mutex> lock(mtx);

			int index = 0;
			int count = links.size();

			for (auto& subLink : links)
			{
				bool wasCreated = client.wordsDoc_exists(subLink);
				if (wasCreated == false)
				{
					tasks.push([subLink, &client, depth, index, count]() { parseLink(subLink, client, depth - 1, index, count); });
					//depth--;
					index++;
				}
				
			}
			cv.notify_one();

		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
}



int main()
{
	try {
		Config config("C:/Users/Firef/source/repos/Webber/config/config.ini");
		Client client (config);

		int depth = atoi(config.getConfig("search_depth").c_str());
		
		std::cout << config.getConfig("db_host") << std::endl;
		std::cout << config.getConfig("db_port") << std::endl;
		std::cout << config.getConfig("db_name") << std::endl;
		std::cout << config.getConfig("db_username") << std::endl;
		std::cout << config.getConfig("db_password") << std::endl;
		std::cout << config.getConfig("url") << std::endl;
		std::cout << config.getConfig("search_depth") << std::endl;
		std::cout << config.getConfig("http_server_port") << std::endl;

		int numThreads = std::thread::hardware_concurrency();
		std::vector<std::thread> threadPool;

		for (int i = 0; i < numThreads; ++i) {
			threadPool.emplace_back(threadPoolWorker);
		}

		//Link link{ ProtocolType::HTTPS, "en.wikipedia.org", "/wiki/Main_Page" }; //en.wikipedia.org/wiki/Main_Page
		Link link(startLinks(config.getConfig("url")));

		{
			std::lock_guard<std::mutex> lock(mtx);
			tasks.push([link, &client, depth]() { parseLink(link, client, depth, 1, 1); });
			cv.notify_one();
		}

		std::this_thread::sleep_for(std::chrono::seconds(2));

		{
			std::lock_guard<std::mutex> lock(mtx);
			exitThreadPool = true;
			cv.notify_all();
		}

		for (auto& t : threadPool) {
			t.join();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}
	return 0;
}
