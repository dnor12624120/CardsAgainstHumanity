#pragma once

/*
	Define to skip a large number of useless windows-related includes.
*/
#define WIN32_LEAN_AND_MEAN

#include <memory>
#include <winsock2.h>

/*
	Singleton manager class for initializing and closing WSA.
*/
class NetworkManager
{
	public:
		/*
			Description:
				Deleted copy constructor.
		*/
		NetworkManager(const NetworkManager&) = delete;
		/*
			Description:
				Deleted copy assignment operator.
		*/
		NetworkManager& operator=(const NetworkManager&) = delete;

		/*
			Description:
				Get-instance function - returns a static std::shared_ptr to an instance of NetworkManager.
		*/
		static std::shared_ptr<NetworkManager> GetInstance();

		/*
			Description:
				Wrapper function that calls WSAStartup.
			Parameters:
				int version - version with which WSA will be initialized.
				int subversion - subversion with which WSA will be initialized.
			Throws:
				std::exception - if WSAStartup was unsuccessful.
		*/
		void Initialize(int version, int subversion);

		/*
			Description:
				Wrapper function that calls WSACleanup.
		*/
		void Cleanup();

		/*
			Description:
				Getter for the version with which WSA was initilized.
		*/
		inline const int GetVersion() const
		{
			return m_version;
		}

		/*
			Description:
				Getter for the subversion with which WSA was initilized.
		*/
		inline const int GetSubversion() const
		{
			return m_subversion;
		}
	private:
		/*
			Description:
				Default constructor for NetworkManager.
		*/
		NetworkManager();

		WSADATA m_data;
		int m_version;
		int m_subversion;
};

