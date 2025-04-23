#pragma once
#include <iostream>

class Connection{
public:
    int GetSocket() const { return conSocket; }

    void SendString(const std::string& message) const;
    void SendVectorOfStrings(const std::vector<std::string>& message) const;
    std::string ReceiveString() const;
    std::vector<std::string> ReceiveVectorOfStrings() const;

private:
    void Send(const void* buffer, size_t size) const;
    void Receive(void* buffer, size_t size) const;

protected:
    int conSocket;
};