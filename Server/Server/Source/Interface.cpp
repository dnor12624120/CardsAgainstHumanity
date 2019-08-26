#include "Interface.h"
#include "Exceptions.h"

#include <iostream>
#include <sstream>

Interface::Interface():
	m_server(std::make_unique<Server>(*this))
{
	setupCommands();
}

void Interface::run()
{
	m_takingInput = true;
	while (m_takingInput)
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
	}
}

void Interface::notify(const std::string& message) 
{
	std::lock_guard<std::mutex> guard(m_consoleGuard);
	std::cout << message << '\n';
}

void Interface::getInput(std::string& input)
{
	std::cout << ">";
	std::getline(std::cin, input);
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
	m_commands["start"] = Command(InterfaceCommand(std::bind(&Interface::startServer, this, std::placeholders::_1)), "start", 0);
}

void Interface::exit(const std::vector<std::string>& arguments)
{
	m_takingInput = false;
}

void Interface::echo(const std::vector<std::string>& arguments)
{
	for (int i = 0; i < arguments.size(); i++)
	{
		std::cout << arguments[i] << ' ';
	}
	std::cout << '\n';
}

void Interface::startServer(const std::vector<std::string>& arguments)
{
	m_server->start();
	m_takingInput = false;
}