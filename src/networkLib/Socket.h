#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

class Socket
{
private:
	sockaddr_in m_addr;

public:

	Socket(const std::string& ip, unsigned short port);
	Socket(unsigned short port);

	static int size();

	std::string ip() const;
	unsigned short port() const;
};