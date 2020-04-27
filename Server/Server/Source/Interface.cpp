#include "Interface.h"
#include "Exceptions.h"

#include <iostream>
#include <sstream>

Interface::Interface():
	server{ std::make_unique<Server>(*this, "127.0.0.1", 11011, "settings.cfg") }
{
	setupCommands();
}

void Interface::run()
{
	takingInput = true;
	while (takingInput)
	{
		std::string streamInput;
		std::string command;
		std::vector<std::string> commandArguments;
		try
		{
			readInputFromConsole(streamInput);
			if (!isInputEmpty(streamInput))
			{
				parseInput(streamInput, command, commandArguments);
				validateInput(command, commandArguments);
				consoleCommands[command](commandArguments);
			}
		}
		catch (InterfaceException& exception)
		{
			printMessage(exception.what());
		}
		catch (CommandException& exception)
		{
			printMessage(exception.what());
		}
	}
}

void Interface::printMessage(const std::string& message) 
{
	std::lock_guard<std::mutex> guard(consoleGuard);
	std::cout << message << '\n';
}

bool Interface::isInputEmpty(const std::string& input)
{
	return input.length() == 0;
}

void Interface::readInputFromConsole(std::string& input)
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
	if (consoleCommands.find(command) == consoleCommands.end())
	{
		throw InterfaceException("Invalid command!");
	}
}

void Interface::setupCommands()
{
	consoleCommands["exit"] = Command(InterfaceCommand(std::bind(&Interface::exit, this, std::placeholders::_1)), "exit", 0);
	consoleCommands["echo"] = Command(InterfaceCommand(std::bind(&Interface::echo, this, std::placeholders::_1)), "echo", cmd::UNLIMITED_ARGUMENTS);
	consoleCommands["start"] = Command(InterfaceCommand(std::bind(&Interface::startServer, this, std::placeholders::_1)), "start", 0);
}

void Interface::exit(const std::vector<std::string>& arguments)
{
	takingInput = false;
}

void Interface::echo(const std::vector<std::string>& arguments)
{
	std::for_each(arguments.begin(), arguments.end(), [](const std::string& argument) { std::cout << argument << ' '; });
	std::cout << '\n';
}

void Interface::startServer(const std::vector<std::string>& arguments)
{
	server->start();
	takingInput = false;
}