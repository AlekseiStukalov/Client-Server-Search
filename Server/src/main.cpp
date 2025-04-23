#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include "input/CommandLineArgs.hpp"
#include "ClientHandler.hpp"
#include "ClientConnection.hpp"
#include "search/SearchEngine.hpp"

const int kDefaultPort = 4000;

int startListen(const CommandLineArgs& args) {
    int serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket < 0) {
        throw std::runtime_error("Unable to create server socket");
    }

    int port = args.m_Port > 0 ? args.m_Port : kDefaultPort;

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    while (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        if (errno == EADDRINUSE) {
            serverAddr.sin_port = htons(++port);
            continue;
        }
        
        throw std::runtime_error("Unable to bind socket: " + std::string(std::strerror(errno)));
    }

    if (listen(serverSocket, SOMAXCONN) < 0){
        throw std::runtime_error("Listen failed: " + std::string(std::strerror(errno)));
    }

    std::cout << "Server listening on port: " << port << std::endl;
    return serverSocket;
}

int main(int argc, char* argv[]) {
    int serverSocket = -1;
    try {
        CommandLineArgs args(argc, argv);

        serverSocket = startListen(args);

        while (true) {
            sockaddr_in clientAddr;
            socklen_t addrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);

            auto thread = std::thread([handler = ClientHandler{}, clientAddr, clientSocket, &searchPath = args.m_SearchPath]() mutable {
                handler.Handle(clientAddr, clientSocket, searchPath);
            });
            
            thread.join();
        }
    }
    catch (const std::exception& e){
        std::cout << "Server exception: " << e.what() << std::endl;

        if (serverSocket >= 0){
            close(serverSocket);
        }
    }

    return 0;
}