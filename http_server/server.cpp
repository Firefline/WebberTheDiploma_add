#include "server.h"
#include "../indexer/indexer.h"


Server::Server(const Config& config) : Database(config)
{
	c.prepare("search",
		"SELECT docs.query, docs.host, sum(wd.total) as total "
		"FROM documents docs "
		"JOIN words_documents wd ON docs.id = wd.document_id "
		"JOIN words w ON wd.word_id = w.id AND w.id = $1 "
		"GROUP BY(docs.query, docs.host) "
		"ORDER BY total limit 10"
	);

	c.prepare("search_one",
		"SELECT docs.query, docs.host, sum(wd.total) as total "
		"FROM documents docs "
		"JOIN words_documents wd ON docs.id = wd.document_id "
		"JOIN words w ON wd.word_id = w.id AND w.id = $1 "
		"group by(docs.query, docs.host) "
		"order by total desc limit 10"
	);

	c.prepare("search_two", 
		"SELECT docs.query, docs.host, sum(wd1.total) + sum(wd2.total) as total "
		"FROM documents docs "
		"JOIN words_documents wd1 ON docs.id = wd1.document_id "
		"JOIN words w1 ON wd1.word_id = w1.id AND w1.id = $1 "

		"JOIN words_documents wd2 ON docs.id = wd2.document_id "
		"JOIN words w2 ON wd2.word_id = w2.id AND w2.id = $2 "
		"group by(docs.query, docs.host) "
		"order by total desc limit 10");

}

std::vector<std::string> Server::searchEngine(const std::string& query)
{
	std::vector<std::string> result;

	auto lowerCase = toLowerCase(query);

	std::vector<std::string> words = indexer(lowerCase);

	pqxx::work txn{ c };

	pqxx::result res;

	if (words.size() == 0)
	{
		return result;
	}
	else if (words.size() == 1)
	{
		res = txn.exec_prepared("search_one", words[0]);
	}
	else if (words.size() == 2)
	{
		res = txn.exec_prepared("search_two", words[0], words[1]);
	}

	for (const auto& row : res) {

		std::string url = "https://";

		url += row["host"].as<std::string>();
		url += row["query"].as<std::string>();
		result.push_back(url);
	}

	return result;

}