#pragma once

#include <exception>
#include <string>

class CommandException : public std::exception
{
	public:
		CommandException(const std::string& message) :
			m_message(message)
		{

		}

		const char* what() const throw()
		{
			return m_message.data();
		}
	private:
		std::string m_message;
};

class InterfaceException : public std::exception
{
	public:
		InterfaceException(const std::string& message) :
			m_message(message)
		{

		}

		const char* what() const throw()
		{
			return m_message.data();
		}
	private:
		std::string m_message;
};

class StdRandGeneratorException : public std::exception
{
	public:
		StdRandGeneratorException(const std::string& message) :
			m_message(message)
		{

		}

		const char* what() const throw()
		{
			return m_message.data();
		}
	private:
		std::string m_message;
};