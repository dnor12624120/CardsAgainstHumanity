#include "Interface.h"

#include <iostream>
#include <sstream>

Interface::Interface():
	client{ std::make_unique<Client>(*this) },
	connected{ false }
{
	setupCommands();
}

void Interface::readInputFromConsole(std::string& input)
{
	std::cout << "> ";
	std::getline(std::cin, input);
}

void Interface::run()
{
	running = true;
	while (running)
	{
		std::string input;
		std::string command;
		std::vector<std::string> arguments;
		try
		{
			readInputFromConsole(input);
			if (input.length())
			{
				parseInput(input, command, arguments);
				validateInput(command, arguments);
				commands[command](arguments);
			}
		}
		catch (InterfaceException& exception)
		{
			std::cout << exception.what() << '\n';
		}
		catch (CommandException& exception)
		{
			std::cout << exception.what() << '\n';
		}
		catch (WinSockException& exception)
		{
			std::cout << "Failed to connect to server: (" << exception.what() << ")\n";
		}
	}
}

void Interface::printMessage(const std::string& message)
{
	std::cout << message << '\n';
}

std::string Interface::requestInput()
{
	std::cout.flush();
	std::cout << ">";
	std::string input;
	std::cin >> input;
	return input;
}

void Interface::parseInput(const std::string& input, std::string& command, std::vector<std::string>& arguments)
{
	std::string argument;
	std::stringstream stringstreamInput(input);

	std::getline(stringstreamInput, command, ' ');

	while (std::getline(stringstreamInput, argument, ' '))
	{
		arguments.emplace_back(argument);
	}
}

void Interface::validateInput(const std::string& command, const std::vector<std::string>& arguments)
{
	if (command.length() == 0)
	{
		throw InterfaceException("No command entered!");
	}
	if (commands.find(command) == commands.end())
	{
		throw InterfaceException("Invalid command!");
	}
}

void Interface::setupCommands()
{
	commands["exit"] = Command(InterfaceCommand(std::bind(&Interface::exit, this, std::placeholders::_1)), "exit", 0);
	commands["echo"] = Command(InterfaceCommand(std::bind(&Interface::echo, this, std::placeholders::_1)), "echo", cmd::UNLIMITED_ARGUMENTS);
	commands["setuser"] = Command(InterfaceCommand(std::bind(&Interface::setUsername, this, std::placeholders::_1)), "setuser", 1);
	commands["connect"] = Command(InterfaceCommand(std::bind(&Interface::connectToServer, this, std::placeholders::_1)), "connect", 0);
}

void Interface::exit(const std::vector<std::string>& arguments)
{
	running = false;
}

void Interface::echo(const std::vector<std::string>& arguments)
{
	for (int i = 0; i < arguments.size(); i++)
	{
		std::cout << arguments[i] << ' ';
	}
	std::cout << '\n';
}

void Interface::setUsername(const std::vector<std::string>& arguments)
{
	client->setUsername(arguments[0]);
}

void Interface::connectToServer(const std::vector<std::string>& arguments)
{
	if (connected)
	{
		throw InterfaceException("Already connected");
	}
	client->connectToServer();
	connected = true;
	running = false;
	client->start();
}