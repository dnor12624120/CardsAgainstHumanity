#pragma once

#include "Exceptions.h"

#include <exception>
#include <functional>
#include <string>
#include <vector>

namespace cmd
{
	const int UNLIMITED_ARGUMENTS = -1;
}

class Command
{
	using InterfaceCommand = std::function<void(const std::vector<std::string>&)>;

	public:
		Command() = default;
		Command(const InterfaceCommand& command, const std::string& commandName, int numberOfArguments);

		void operator()(const std::vector<std::string>& arguments);

		inline int getNumberOfArguments() const { return numberOfArguments; }
	private:
		InterfaceCommand command;
		std::string commandName;
		int numberOfArguments;
};