#pragma once
#include <thread>
#include "Config.hpp"
#include "../../Common/Connection.hpp"

class ServerConnection : public Connection {
public:
    ServerConnection(const IpAddr& addr);
    ~ServerConnection();

    ServerConnection(const ServerConnection& other) = delete;
    ServerConnection& operator=(const ServerConnection& other) = delete;
    
    ServerConnection(ServerConnection&& other) = delete;
    ServerConnection& operator=(ServerConnection&& other) = delete;

    void Connect();

private:
    void Ping() const;

    std::atomic_bool m_IsAlive;
    IpAddr m_Address;
    std::thread m_PingThread;
};