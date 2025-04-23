#include "ClientConnection.hpp"
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

ClientConnection::~ClientConnection(){
    if (conSocket >= 0){
        close(conSocket);
        conSocket = -1;
    }

    m_IsAlive.store(false);
}

ClientConnection::ClientConnection(const ClientConnection &other) {
    m_ClientIpStr = std::move(other.m_ClientIpStr);
    conSocket = other.conSocket;
}

ClientConnection& ClientConnection::operator=(const ClientConnection &other){
    if (this != &other) {
        m_ClientIpStr = std::move(other.m_ClientIpStr);
        conSocket = other.conSocket;
    }
    return *this;
}

ClientConnection::ClientConnection(ClientConnection &&other) noexcept {
    m_ClientIpStr = std::move(other.m_ClientIpStr);
    conSocket = other.conSocket;
    other.conSocket = -1;
}

ClientConnection& ClientConnection::operator=(ClientConnection &&other) noexcept {
    if (this != &other) {
        m_ClientIpStr = std::move(other.m_ClientIpStr);
        conSocket = other.conSocket;
        other.conSocket = -1;
    }
    return *this;
}

void ClientConnection::Initialise(int socket, const sockaddr_in &clientAddr){
    conSocket = socket;

    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    m_ClientIpStr = std::string(ipStr);

    timeval timeout;
    timeout.tv_sec = 10;
    setsockopt(conSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    m_IsAlive.store(true);
}
