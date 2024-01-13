#pragma once

#include "../database/database.h"
#include "link.h"
#include <mutex>
#include <regex>

class Client : public Database
{
public:
	Client(const Config& config);

	void wordsDoc_new(const Link& link, const std::map<std::string, int>& wordsTotal);
	bool wordsDoc_exists(const Link& link);

protected:
	std::mutex mutex_;
};