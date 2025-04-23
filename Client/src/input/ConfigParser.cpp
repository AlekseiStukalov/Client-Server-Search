#include <fstream>
#include <iostream>
#include <filesystem>
#include "ConfigParser.hpp"
#include "JsonParser.hpp"

void ConfigParser::LoadConfig(const CommandLineArgs &args, Config &config) const {
    std::filesystem::path configPath;

    if (args.m_ConfigPathStr.empty()){
        configPath = args.m_CurrentDir / std::filesystem::path(defaultPath);
    }
    else {
        std::filesystem::path path{args.m_ConfigPathStr};
        if (!path.is_absolute()){
            path = path.filename();
            configPath = args.m_CurrentDir / std::filesystem::path(defaultPath);
        }
        else {
            configPath = path;
        }
    }

    std::ifstream fs{configPath};

    if (!fs){
        throw std::runtime_error("Cannot open config file: " + configPath.string());
    }

    JsonParser parser;
    parser.Parse(fs);
    config.patterns = parser.GetSearchWords();
    std::vector<std::string> addresses = parser.GetServers();

    for (const std::string& addr : addresses){
        config.addresses.emplace_back(ParseAddr(addr));
    }

    std::cout << "Config file loaded. Path: " << configPath << std::endl;
}

IpAddr ConfigParser::ParseAddr(const std::string &addr) const
{
    int colonIdx = addr.find(":");
    if (colonIdx == -1){
        throw std::invalid_argument("Wrong server address: " + addr);
    }

    std::string ipStr{addr.begin(), addr.begin() + colonIdx};
    int port = std::stoi(std::string{addr.begin() + colonIdx + 1, addr.end()});

    return IpAddr {ipStr, port};
}
