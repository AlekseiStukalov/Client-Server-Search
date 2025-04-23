#include "ClientResponse.hpp"

void ClientResponse::Initialise(){
    m_IsInWork.store(true);
    m_ShouldExit.store(false);
    m_Thread = std::thread(&ClientResponse::SendThreadWorker, this);
}

void ClientResponse::SendMessage(const std::string &message) {
    m_Lock.lock();
    m_SendQueue.push(message);
    m_Lock.unlock();

    m_IdleCondition.notify_one();
}

void ClientResponse::WaitForExit() {
    m_ShouldExit.store(true);
    m_IdleCondition.notify_one();
    m_Thread.join();
}

void ClientResponse::SendThreadWorker(){
    try{
        while (true){
            std::string message;
            std::unique_lock<std::mutex> lock {m_Lock};
            m_IdleCondition.wait(lock, [&]() { return !m_SendQueue.empty() || !m_Connection.IsAlive() || m_ShouldExit.load(); });
    
            if (m_ShouldExit.load() || !m_Connection.IsAlive()) return;
    
            if (!m_SendQueue.empty()){
                message = std::move(m_SendQueue.front());
                m_SendQueue.pop();
            }
            lock.unlock();
    
            m_Connection.SendString(message);
        }
    }
    catch (const std::exception& e) {
        std::cout << "Failed to send message to the client " << m_Connection.GetClientIp() << std::endl;
    }
    m_IsInWork.store(false);
}
