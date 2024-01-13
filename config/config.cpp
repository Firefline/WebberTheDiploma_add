#include "config.h"

Config::Config(const std::string& filename)
{
    Config::parser(filename);

}

std::string Config::getConfig(const std::string& key) const
{
    if (Config::configs.empty())
    {
        throw std::runtime_error("ERROR: Empty config file.");
    }
    return Config::configs.at(key);
}

void Config::parser(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("ERROR: File cannot be open");
    }
    std::string temp;
    while (!file.eof())
    {
        std::getline(file, temp);
        std::size_t eqPos = temp.find("=");
        if (eqPos != std::string::npos)
        {
            Config::configs.insert(std::make_pair<std::string, std::string>(temp.substr(0, eqPos), temp.substr(eqPos + 1, std::string::npos)));
        }
    }

    file.close();
}