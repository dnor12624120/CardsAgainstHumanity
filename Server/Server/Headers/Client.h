#pragma once

#include "WNetwork.h"
#include <string>

class Client
{
	public:
		Client() :
			m_socket(Socket(Family_IPv4, SocketType_Stream, Protocol_TCP)),
			m_score(0)
		{

		}

		inline void setUsername(const std::string& username) { m_username = username; }
		inline Socket& getSocket() { return m_socket; }
		inline IPv4Address& getAddress() { return m_address; }
		inline const std::string& getUsername() const { return m_username; }
		inline int getScore() const { return m_score; }
		inline void incrementScore() { m_score++; }
		inline bool operator==(const Client& client) { return m_username == client.m_username; }
	private:
		std::string m_username;
		Socket m_socket;
		IPv4Address m_address;
		int m_score;
};