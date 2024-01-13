#include "indexer.h"
#include <boost/locale.hpp>
#include <regex>

//using namespace boost::locale;

std::string toLowerCase(const std::string& word)
{
	using namespace boost::locale;
	generator gen;
	std::locale loc = gen("");
	std::locale::global(loc);

	auto result = to_lower(word);

	return result;

}

std::vector<std::string> indexer(const std::string& rawData)
{
	std::vector<std::string> result;
	std::string delimiters(" \r\n\t.,;:!&()[]{}\"/=+-*'");

	size_t index = 0;
	std::string temp;

	while (index < rawData.size())
	{
		if (delimiters.find(rawData[index]) == std::string::npos)
		{
			temp += rawData[index];
		}
		else
		{
			if (temp.size() > 0)
			{
				result.push_back(temp);
			}
			temp = "";
		}
		index++;
	}

	if (temp.size() > 0)
	{
		result.push_back(temp);
	}

	return result;
}