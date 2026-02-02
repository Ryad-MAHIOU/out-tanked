#include "Address.h"
#include <cstring>

Address::Address(const std::string& ip, unsigned short port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);

	inet_pton(AF_INET, ip.c_str(), &m_addr.sin_addr);
}

Address::Address(unsigned short port)
{
	memset(&m_addr, 0, sizeof(m_addr));
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);
	m_addr.sin_addr.s_addr = INADDR_ANY;
}

int Address::size()
{
	return sizeof(sockaddr_in);
}

std::string Address::ip() const
{
	char ipStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &m_addr.sin_addr, ipStr, INET_ADDRSTRLEN);
	return std::string(ipStr);
}

unsigned short Address::port() const
{
	return ntohs(m_addr.sin_port);
}