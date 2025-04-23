#include "ServerConnection.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

ServerConnection::ServerConnection(const IpAddr &addr){
    conSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (conSocket < 0) {
        throw std::runtime_error("Unable to create socket while connecting to server " + addr.ipStr + ":" + std::to_string(addr.port));
    }

    timeval timeout;
    timeout.tv_sec = 10;
    setsockopt(conSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    //SO_RCVTIMEO - unlim

    m_IsAlive.store(false);
    m_Address = addr;
}

ServerConnection::~ServerConnection(){
    m_IsAlive.store(false);
    
    if (m_PingThread.joinable()) {
        m_PingThread.join();
    }

    if (conSocket >= 0){
        close(conSocket);
        conSocket = -1;
    }
}

void ServerConnection::Connect(){
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(m_Address.port);
    inet_pton(AF_INET, m_Address.ipStr.c_str(), &serverAddr.sin_addr);

    if (connect(conSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(conSocket);
        throw std::runtime_error("Unable to connect to server " + m_Address.ipStr + ":" + std::to_string(m_Address.port));
    }

    m_IsAlive.store(true);
    m_PingThread = std::thread(&ServerConnection::Ping, this);
}

void ServerConnection::Ping() const {
    try{
        while (m_IsAlive.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            SendString("PING");
        }
    }
    catch (const std::exception& e){}
}
