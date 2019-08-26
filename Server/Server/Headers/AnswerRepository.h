#pragma once

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

class AnswerRepository
{
	public:
		AnswerRepository() = default;
		AnswerRepository(const std::string& filepath);

		const std::string& getAnswer();
	private:
		std::string m_filepath;
		std::vector<std::string> m_answers;
		int m_counter;
};