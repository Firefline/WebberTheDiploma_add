#pragma once

#include <iostream>
#include <vector>
#include "../database/database.h"
#include "../indexer/indexer.h"

class Server : public Database
{
public:
	Server(const Config& config);

	std::vector<std::string> searchEngine(const std::string& query);

};