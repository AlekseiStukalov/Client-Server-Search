#pragma once
#include <string>
#include <vector>

struct IpAddr {
    std::string ipStr;
    int port;
};

struct Config {
    std::vector<std::string> patterns;
    std::vector<IpAddr> addresses;
};
