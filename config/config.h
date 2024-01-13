#pragma once

#include <string>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <exception>

class Config
{
public:
    Config() {};
    Config(const std::string&);

    std::string getConfig(const std::string& key) const;

protected:
    std::unordered_map<std::string, std::string> configs;

    void parser(const std::string& filename);
};