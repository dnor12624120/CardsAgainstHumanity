#include "Command.h"

Command::Command(const InterfaceCommand& command, const std::string& commandName, int numberOfArguments) :
	command{ command },
	commandName{ commandName },
	numberOfArguments{ numberOfArguments }
{

}

void Command::operator()(const std::vector<std::string>& arguments)
{
	if (arguments.size() != numberOfArguments && numberOfArguments != cmd::UNLIMITED_ARGUMENTS)
	{
		throw CommandException("Invalid number of arguments!");
	}
	command(arguments);
}
