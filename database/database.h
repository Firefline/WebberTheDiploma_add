#pragma once

#include <pqxx/pqxx>
#include "../config/config.h"

class Database
{
protected:
	pqxx::connection c;

public:
	Database(const Config& config);

	void tableCreator();
};