#pragma once

#include "Command.h"
#include "Server.h"

#include <istream>
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
		void printMessage(const std::string& message);
	private:
		void setupCommands();

		void readInputFromConsole(std::string& input);
		void parseInput(const std::string& input, std::string& command, std::vector<std::string>& arguments);
		void validateInput(const std::string& command, const std::vector<std::string>& arguments);

		void exit(const std::vector<std::string>& arguments);
		void echo(const std::vector<std::string>& arguments);
		void startServer(const std::vector<std::string>& arguments);

		bool isInputEmpty(const std::string& input);


		bool takingInput;

		std::map<std::string, Command> consoleCommands;

		std::mutex consoleGuard;

		std::unique_ptr<Server> server;

};