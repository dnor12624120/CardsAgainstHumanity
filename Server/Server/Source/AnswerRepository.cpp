#include "AnswerRepository.h"

#include <algorithm>
#include <ctime>
#include <random>

AnswerRepository::AnswerRepository(const std::string& filepath) :
	m_filepath(filepath),
	m_counter(0)
{
	srand(time(0));
	std::ifstream inputFile(m_filepath);
	std::string answer;
	while (std::getline(inputFile, answer))
	{
		m_answers.emplace_back(answer);
	}
	std::random_shuffle(m_answers.begin(), m_answers.end(), [](int i){ return rand() % i; });
	inputFile.close();
}

const std::string& AnswerRepository::getAnswer()
{
	return m_answers[m_counter++];
}