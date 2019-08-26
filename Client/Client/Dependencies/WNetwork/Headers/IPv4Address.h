#pragma once

#include "Types.h"

#include <WinSock2.h>
#include <string>
#include <vector>

class IPv4Address
{
	public:
		IPv4Address() = default;

		IPv4Address(const std::string& ip, unsigned short port);

		IPv4Address(SocketAddress* address);

		InSocketAddress GetSocketAddress() const;

		inline const std::string& GetHostname() const { return m_hostname; }

		inline const std::string& GetIP() const { return m_ip; }

		inline const std::vector<unsigned char>& GetIPBytes() const { return m_ipBytes; }

		inline const unsigned char GetPort() const { return m_port; }
	private:
		std::string m_hostname;
		std::string m_ip;
		std::vector<unsigned char> m_ipBytes;

		unsigned short m_port;
};