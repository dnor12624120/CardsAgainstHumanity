#pragma once

#define WIN32_LEAN_AND_MEAN

#define DEFAUKLT_BACKLOG 8

#include "Types.h"

#include <WinSock2.h>

class IPv4Address;

class Socket
{
	public:
		Socket() = default;
		/*
			Description: Creates a new socket object with specified fields.
			Parameters:
				SocketAddressFamily family - 
				SocketType type -
				SocketProtocol protocol -
		*/
		Socket(SocketAddressFamily family, SocketType type, SocketProtocol protocol);

		/*
		
		*/
		Socket(SocketHandle handle, SocketAddressFamily family, SocketType type, SocketProtocol protocol);

		/*
			Description:
			Throws:
		*/
		void Create();

		/*
			Description:
			Throws:
		*/
		void Close();

		/*
			Description:
			Parameters:
				bool on - indicates whether Nagle's algorithm should be switched on/off.
			Throws:
		*/
		void ToggleNagle(bool on);

		/*
			Description:
			Parameters:
		*/
		void Bind(const IPv4Address& address);

		/*
			Description:
			Parameters:
		*/
		void Listen();

		/*
		
		*/
		void Accept(Socket& recievedSocket, IPv4Address& socketAddress);

		/*
		
		*/
		void Connect(const IPv4Address& address);

		/*
			Description: setter function for the listen backlog. 
			Parameters: new backlog number to be set.
		*/

		/*
		
		*/
		void SendAll(const void* data, int size);

		/*
		
		*/
		void RecieveAll(void* data, int size);
		
		/*
		
		*/
		inline void SetBacklog(int backlog) { m_backlog = backlog; }

		/*
			Description:
			Returns:
		*/
		inline const SocketHandle GetHandle() const { return m_handle; }

		/*
			Description:
			Returns:
		*/
		inline const SocketAddressFamily GetFamily() const { return m_family; }

		/*
			Description:
			Returns:
		*/
		inline const SocketType GetType() const { return m_type; }

		/*
			Description:
			Returns:
		*/
		inline const SocketProtocol GetProtocol() const { return m_protocol; }

		inline const bool operator==(const Socket& other) { return m_handle == other.m_handle; }
	private:
		/*

		*/
		int Send(const void* data, int size);

		/*

		*/
		int Recieve(void* data, int size);


		SocketHandle m_handle;
		SocketAddressFamily m_family;
		SocketType m_type;
		SocketProtocol m_protocol;
		int m_backlog;
};