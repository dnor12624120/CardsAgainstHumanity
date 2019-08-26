#pragma once

#include "Command.h"
#include "Server.h"

#include <map>
#include <mutex>
#include <string>
#include <vector>

class Interface
{
	using InterfaceCommand = std::function<void(const std::vector<std::string>&)>;

	public:
		Interface();

		void run();
		void notify(const std::string& message);
	private:
		void setupCommands();

		void getInput(std::string& input);
		void parseInput(const std::string& input, std::string& command, std::vector<std::string>& arguments);
		void validateInput(const std::string& command, const std::vector<std::string>& arguments);

		void exit(const std::vector<std::string>& arguments);
		void echo(const std::vector<std::string>& arguments);
		void startServer(const std::vector<std::string>& arguments);

		bool m_takingInput;
		std::map<std::string, Command> m_commands;
		std::mutex m_consoleGuard;
		std::unique_ptr<Server> m_server;

};