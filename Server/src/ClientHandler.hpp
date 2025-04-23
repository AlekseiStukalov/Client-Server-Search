#pragma once
#include <string>
#include <filesystem>
#include "ClientConnection.hpp"
#include "search/SearchEngine.hpp"

class ClientHandler {
public:
    ClientHandler(){};
    void Handle(sockaddr_in clientAddr, int clientSocket, const std::string& searchPath);

private:
    std::unordered_set<std::string> DistinctClientInput(const std::vector<std::string>& vec);
};