#pragma once
#include <string>
#include "CommandLineArgs.hpp"
#include "../Config.hpp"

class ConfigParser {
public:
    ConfigParser() : defaultPath {"config.json"} {};
    
    void LoadConfig(const CommandLineArgs& args, Config& config) const;

private:
    IpAddr ParseAddr(const std::string& addr) const;

    const std::string defaultPath;
};