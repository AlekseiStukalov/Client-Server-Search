#include "ClientHandler.hpp"
#include "ClientResponse.hpp"

void ClientHandler::Handle(sockaddr_in clientAddr, int clientSocket, const std::string& searchPath) {
    ClientConnection cc;
    ClientResponse responseManager(cc);
    SearchEngine searchEngine(searchPath, responseManager);

    try {
        cc.Initialise(clientSocket, clientAddr);
        std::cout << "Client connected from " << cc.GetClientIp() << std::endl;

        responseManager.Initialise();

        std::unordered_set<std::string> patterns = DistinctClientInput(cc.ReceiveVectorOfStrings());
        std::thread(&SearchEngine::Run, &searchEngine, patterns).detach();

        while (true){
            std::string rcv = cc.ReceiveString();
            if (rcv == "PING"){
                //std::cout << cc.GetClientIp() << " : Ping" << std::endl;
            }

            if (!responseManager.IsWorking()) throw;
        }
    }
    catch (const std::exception& e) {
        cc.Close();
        responseManager.WaitForExit();
        searchEngine.WaitForExit();
        std::cout << "Client " << cc.GetClientIp() << " was disconnected" << std::endl;
    }
}

std::unordered_set<std::string> ClientHandler::DistinctClientInput(const std::vector<std::string> &vec) {
    std::unordered_set<std::string> result;
    for (const std::string& str : vec) {
        std::string lower = str;
        std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
        result.insert(std::move(lower));
    }
    return result;
}
