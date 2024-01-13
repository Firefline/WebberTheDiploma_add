#include "database.h"

Database::Database(const Config& config) : c(
	" host=" + config.getConfig("db_host") +
	" port=" + config.getConfig("db_port") +
	" dbname=" + config.getConfig("db_name") +
	" user=" + config.getConfig("db_username") +
	" password=" + config.getConfig("db_password"))
	{
	tableCreator();
	}

void Database::tableCreator()
{
	pqxx::work table{ c };

	table.exec("CREATE TABLE IF NOT EXISTS words (id varchar(255) not null primary key); ");
	table.exec("CREATE TABLE IF NOT EXISTS documents (id serial primary key, host text not null, query text not null); ");
	table.exec("CREATE TABLE IF NOT EXISTS words_documents (id serial primary key, word_id varchar(255) not null references words(id), document_id integer not null references documents(id), total integer not null); ");
	table.commit();
}