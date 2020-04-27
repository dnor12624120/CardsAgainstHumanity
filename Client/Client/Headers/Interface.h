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
		
		void printMessage(const std::string& message);
		std::string requestInput();
	private:
	     void readInputFromConsole(std::string& input);
		 void parseInput(const std::string& input, std::string& command, std::vector<std::string>& arguments);
		 void validateInput(const std::string& command, const std::vector<std::string>& arguments);

		void setupCommands();

		void exit(const std::vector<std::string>& arguments);
		void echo(const std::vector<std::string>& arguments);
		void setUsername(const std::vector<std::string>& arguments);
		void connectToServer(const std::vector<std::string>& arguments);

		bool running;
		bool connected;

		std::map<std::string, Command> commands;
		std::unique_ptr<Client> client;
};