#pragma once

#include <string>
#include <iostream>

class Question
{
	public:
		Question() = default;
		Question(const std::string& text, int numOfBlanks) :
			m_text(text),
			m_numOfBlanks(numOfBlanks)
		{

		}

		inline friend std::istream& operator>>(std::istream& stream, Question& question)
		{
			std::string numOfBlanks;
			std::getline(stream, question.m_text);
			std::getline(stream, numOfBlanks);
			question.m_numOfBlanks = atoi(numOfBlanks.c_str());
			return stream;
		}

		inline friend std::ostream& operator<<(std::ostream& stream, const Question& question)
		{
			stream << question.m_text << '\n';
			stream << question.m_numOfBlanks << '\n';
			return stream;
		}

		std::string m_text;
		int m_numOfBlanks;
};
