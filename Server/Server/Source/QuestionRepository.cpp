#include "QuestionRepository.h"

#include <algorithm>
#include <ctime>
#include <random>

QuestionRepository::QuestionRepository(const std::string& filepath) :
	m_filepath(filepath),
	m_counter(0)
{
	srand(time(0));
	std::ifstream inputFile(m_filepath);
	Question question;
	while (inputFile >> question)
	{
		m_questions.emplace_back(question);
	}
	std::random_shuffle(m_questions.begin(), m_questions.end(), [](int i){ return rand() % i; });
	inputFile.close();
}

const Question& QuestionRepository::getQuestion()
{
	return m_questions[m_counter++];
}