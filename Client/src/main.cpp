#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include "Config.hpp"
#include "input/ConfigParser.hpp"
#include "input/CommandLineArgs.hpp"
#include "ServerConnection.hpp"

std::mutex consoleSync;

void log(const IpAddr& serverAddr, const std::string& message){
    std::lock_guard<std::mutex> sync(consoleSync);
    std::cout << "[" << serverAddr.ipStr << ":" << serverAddr.port << "] : " << message << std::endl;
}

void workWithServer(const IpAddr& serverAddr, const std::vector<std::string>& patterns){
    try {
        ServerConnection sc(serverAddr);
        sc.Connect();
        log(serverAddr, "Connected!");

        sc.SendVectorOfStrings(patterns);

        while (true) {
            std::string data = sc.ReceiveString();
            log(serverAddr, data);
        }
    }
    catch (const std::exception& e){
        log(serverAddr, "[FAIL] " + std::string(e.what()));
    }
}

int main(int argc, char* argv[]) {
    try {
        CommandLineArgs args{argc, argv};
        Config config;
        ConfigParser parser;
        parser.LoadConfig(args, config);

        std::vector<std::thread> serverThreads;
        for (const IpAddr& serverAddr : config.addresses){
            serverThreads.emplace_back(std::thread(workWithServer, serverAddr, std::cref(config.patterns)));
        }

        for (std::thread& t : serverThreads){
            t.join();
        }
    }
    catch(const std::exception& e){
        std::cout << e.what() << std::endl;
        std::cout << "Client program terminated due to exception. " << std::endl;
    }

    std::cout << "Press enter to exit." << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
    return 0;
}