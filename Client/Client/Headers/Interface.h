#pragma once

#include "Client.h"
#include "Command.h"

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Interface
{
	using InterfaceCommand = std::function<void(const std::vector<std::string>&)>;

	public:
		Interface();

		void run();
		
		void notify(const std::string& message);
		std::string requestInput();
	private:
		void getInput(std::string& input);
		void parseInput(const std::string& input, std::string& command, std::vector<std::string>& arguments);
		void validateInput(const std::string& command, const std::vector<std::string>& arguments);

		void setupCommands();

		void exit(const std::vector<std::string>& arguments);
		void echo(const std::vector<std::string>& arguments);
		void setUsername(const std::vector<std::string>& arguments);
		void connectToServer(const std::vector<std::string>& arguments);

		bool m_running;
		bool m_connected;

		std::map<std::string, Command> m_commands;
		std::unique_ptr<Client> m_client;
};