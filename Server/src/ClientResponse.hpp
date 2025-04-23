#pragma once
#include <iostream>
#include <thread>
#include <atomic>
#include "ClientConnection.hpp"

class ClientResponse{
public:
    ClientResponse(const ClientConnection& connection) : m_Connection{connection} {};

    void Initialise();

    void SendMessage(const std::string& message);
    void WaitForExit();
    bool IsWorking() const { return m_IsInWork.load(); }
    std::string GetClientIp() const { return m_Connection.GetClientIp(); }

private:
    void SendThreadWorker();

    std::atomic_bool m_IsInWork;
    std::atomic_bool m_ShouldExit;
    std::mutex m_Lock;
    std::queue<std::string> m_SendQueue;
    const ClientConnection& m_Connection;
    std::condition_variable m_IdleCondition;
    std::thread m_Thread;
};