#pragma once
#include <string>
#include <atomic>
#include <netinet/in.h>
#include "../../Common/Connection.hpp"

class ClientConnection : public Connection {
public:
    ClientConnection(){};
    ~ClientConnection();
    ClientConnection(const ClientConnection& other);
    ClientConnection& operator=(const ClientConnection& other);

    ClientConnection(ClientConnection&& other) noexcept;
    ClientConnection& operator=(ClientConnection&& other) noexcept;

    std::string GetClientIp() const { return m_ClientIpStr; }
    bool IsAlive() const { return m_IsAlive.load(); }
    void Close() { m_IsAlive.store(false); }

    void Initialise(int socket, const sockaddr_in& clientAddr);

private:
    std::atomic_bool m_IsAlive;
    std::string m_ClientIpStr;
};