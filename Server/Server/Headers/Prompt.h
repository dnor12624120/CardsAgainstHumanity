#pragma once

#include <string>
#include <iostream>

class Prompt
{
	public:
		Prompt() = default;
		Prompt(const std::string& text, int numOfBlanks) :
			text{ text },
			numOfBlanks{ numOfBlanks }
		{

		}

		inline friend std::istream& operator>>(std::istream& inputStream, Prompt& prompt)
		{
			std::string numOfBlanks;
			std::getline(inputStream, prompt.text);
			std::getline(inputStream, numOfBlanks);
			prompt.numOfBlanks = atoi(numOfBlanks.c_str());
			return inputStream;
		}

		inline friend std::ostream& operator<<(std::ostream& outputStream, const Prompt& prompt)
		{
			outputStream << prompt.text << '\n';
			outputStream << prompt.numOfBlanks << '\n';
			return outputStream;
		}

		std::string text;
		int numOfBlanks;
};
