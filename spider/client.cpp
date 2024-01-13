#include "client.h"

Client::Client(const Config& config) : Database(config)
{
	c.prepare("insert_word", "INSERT INTO words (id) VALUES ($1) ON CONFLICT (id) DO NOTHING;");
	c.prepare("insert_document", "INSERT INTO documents (host, query) VALUES ($1, $2) RETURNING id;");
	c.prepare("insert_word_document", "INSERT INTO words_documents (word_id, document_id, total) VALUES ($1, $2, $3) RETURNING id;");
	c.prepare("document_counter", "SELECT COUNT(*) FROM documents WHERE host=$1 AND query=$2");

}

void Client::wordsDoc_new(const Link& link, const std::map<std::string, int>& wordsTotal)
{
	std::lock_guard<std::mutex> lock(mutex_);

	bool wasCreated = wordsDoc_exists(link);
	if (wasCreated == true)
	{
		return;
	}
	
	pqxx::work doc_trx{ c };
	pqxx::result res = doc_trx.exec_prepared("insert_document", link.hostName, link.query);
	doc_trx.commit();

	if (!res.empty()) {
		int id = res[0][0].as<int>();

		pqxx::work words_trx{ c };

		for (auto w : wordsTotal)
		{
			words_trx.exec_prepared("insert_word", w.first);
			words_trx.exec_prepared("insert_word_document", w.first, id, w.second);
		}

		words_trx.commit();
	}

}

bool Client::wordsDoc_exists(const Link& link)
{
	pqxx::work doc_trx{ c };
	pqxx::result res = doc_trx.exec_prepared("document_counter", link.hostName, link.query);
	doc_trx.commit();

	if (!res.empty())
	{
		int id = res[0][0].as<int>();

		if (id == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	return false;

}