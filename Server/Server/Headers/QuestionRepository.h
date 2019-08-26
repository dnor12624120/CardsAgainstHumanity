#pragma once

#include "Question.h"

#include <algorithm>
#include <fstream>
#include <map>
#include <string>
#include <vector>

class QuestionRepository
{
	public:
		QuestionRepository() = default;
		QuestionRepository(const std::string& filepath);

		const Question& getQuestion();
	private:
		std::string m_filepath;
		std::vector<Question> m_questions;
		int m_counter;
};