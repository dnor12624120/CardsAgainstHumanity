#include "Interface.h"

#include <iostream>
#include <sstream>

Interface::Interface():
	m_client(std::make_unique<Client>(*this)),
	m_connected(false)
{
	setupCommands();
}

void Interface::getInput(std::string& input)
{
	std::cout << "> ";
	std::getline(std::cin, input);
}

void Interface::run()
{
	m_running = true;
	while (m_running)
	{
		std::string input;
		std::string command;
		std::vector<std::string> arguments;
		try
		{
			getInput(input);
			if (input.length())
			{
				parseInput(input, command, arguments);
				validateInput(command, arguments);
				m_commands[command](arguments);
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
		catch (WNException& exception)
		{
			std::cout << "Failed to connect to server: (" << exception.what() << ")\n";
		}
	}
}

void Interface::notify(const std::string& message)
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
	if (m_commands.find(command) == m_commands.end())
	{
		throw InterfaceException("Invalid command!");
	}
}

void Interface::setupCommands()
{
	m_commands["exit"] = Command(InterfaceCommand(std::bind(&Interface::exit, this, std::placeholders::_1)), "exit", 0);
	m_commands["echo"] = Command(InterfaceCommand(std::bind(&Interface::echo, this, std::placeholders::_1)), "echo", cmd::UNLIMITED_ARGUMENTS);
	m_commands["setuser"] = Command(InterfaceCommand(std::bind(&Interface::setUsername, this, std::placeholders::_1)), "setuser", 1);
	m_commands["connect"] = Command(InterfaceCommand(std::bind(&Interface::connectToServer, this, std::placeholders::_1)), "connect", 0);
}

void Interface::exit(const std::vector<std::string>& arguments)
{
	m_running = false;
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
	m_client->setUsername(arguments[0]);
}

void Interface::connectToServer(const std::vector<std::string>& arguments)
{
	if (m_connected)
	{
		throw InterfaceException("Already connected");
	}
	m_client->connectToServer();
	m_connected = true;
	m_running = false;
	m_client->start();
}