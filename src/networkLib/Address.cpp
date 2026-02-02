#include "Socket.h"
#include <cstring>

Socket::Socket(const std::string& ip, unsigned short port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);

	inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
}

Socket::Socket(unsigned short port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
}

int Socket::size()
{
	return sizeof(sockaddr_in);
}

std::string Socket::ip() const
{
	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &m_addr.sin_addr, ipStr, INET_ADDRSTRLEN);
	return std::string(ipStr);
}

unsigned short Socket::port() const
{
	return ntohs(m_addr.sin_port);
}