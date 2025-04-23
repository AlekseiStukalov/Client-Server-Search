#include "Connection.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void Connection::SendString(const std::string &message) const{
    uint32_t size = htonl(message.size());
    Send(&size, sizeof(size));
    Send(message.data(), message.size());
}

void Connection::SendVectorOfStrings(const std::vector<std::string> &vec) const{
    uint32_t vecSize = htonl(vec.size());
    Send(&vecSize, sizeof(vecSize));

    for (const std::string& str : vec) {
        SendString(str);
    }
}

std::string Connection::ReceiveString() const{
    uint32_t size = 0;
    Receive(&size, sizeof(size));
    size = ntohl(size);

    std::string res(size, '\0');
    Receive(&res[0], size);
    return res;
}

std::vector<std::string> Connection::ReceiveVectorOfStrings() const{
    uint32_t vecSize = 0;
    Receive(&vecSize, sizeof(vecSize));
    vecSize = ntohl(vecSize);

    std::vector<std::string> res;
    res.reserve(vecSize);
    for (uint32_t i = 0; i < vecSize; ++i) {
        res.push_back(ReceiveString());
    }
    return res;
}

void Connection::Send(const void* buffer, size_t size) const{
    const char* ptr = static_cast<const char*>(buffer);
    ssize_t total = 0;
    while (total < size) {
        ssize_t sent = send(conSocket, ptr + total, size - total, MSG_NOSIGNAL);
        if (sent > 0){
            total += sent;
        }
        else if (sent < 0 && errno == EINTR) {
            continue;
        }
        else {
            throw std::runtime_error("Failed to send data. Considering as disconnected. Error: " + std::string(std::strerror(errno)));
        }
    }
}

void Connection::Receive(void* buffer, size_t size) const {
    char* ptr = static_cast<char*>(buffer);
    size_t total = 0;
    while (total < size) {
        ssize_t received = recv(conSocket, ptr + total, size - total, 0);
        if (received > 0){
            total += received;
        }
        else if (received < 0 && errno == EINTR){
            continue;
        }
        else {
            throw std::runtime_error("Failed to receive data. Considering as disconnected. Error: " + std::string(std::strerror(errno)));
        }
    }
}
