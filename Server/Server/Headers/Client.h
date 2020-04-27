#pragma once

#include "WNetwok.h"

#include <string>

class Client
{
	public:
		Client() :
			socket{ Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) },
			score{ 0 }
		{

		}

		inline void setUsername(const std::string& username) { this->username = username; }
		inline Socket& getSocket() { return socket; }
		inline IPv4Address& getAddress() { return address; }
		inline const std::string& getUsername() const { return username; }
		inline int getScore() const { return score; }
		inline void incrementScore() { score++; }
		inline bool operator==(const Client& client) { return username == client.username; }
	private:
		std::string username;
		Socket socket;
		IPv4Address address;
		int score;
};