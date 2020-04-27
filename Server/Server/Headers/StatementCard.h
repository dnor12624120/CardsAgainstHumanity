#pragma once

#include <string>
#include <iostream>

class StatementCard
{
	public:
	   StatementCard() = default;
	    StatementCard(const std::string& text) :
		    text{ text }
	    {

    	}

		inline friend std::istream& operator>>(std::istream& inputStream, StatementCard& card)
		{
			std::getline(inputStream, card.text);
			return inputStream;
		}

		inline friend std::ostream& operator<<(std::ostream& outputStream, const StatementCard& card)
		{
			outputStream << card.text << '\n';
			return outputStream;
		}

		std::string text;
};
