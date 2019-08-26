#include "Command.h"

Command::Command(const InterfaceCommand& command, const std::string& commandName, int numberOfArguments) :
	m_command(command),
	m_commandName(commandName),
	m_numberOfArguments(numberOfArguments)
{

}

void Command::operator()(const std::vector<std::string>& arguments)
{
	if (arguments.size() != m_numberOfArguments && m_numberOfArguments != cmd::UNLIMITED_ARGUMENTS)
	{
		throw CommandException("Invalid number of arguments!");
	}
	m_command(arguments);
}
