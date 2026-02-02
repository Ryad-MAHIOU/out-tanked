#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class Address
{
    friend class UdpSocket;
private:
    sockaddr_in m_addr;

public:
    Address();
    Address(const std::string& ip, unsigned short port);
    Address(unsigned short port);
    Address(sockaddr_in addr);

    std::string GetIP() const;
    unsigned short GetPort() const;
};