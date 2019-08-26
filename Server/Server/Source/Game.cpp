#include "Game.h"

#include <algorithm>
#include <ctime>
#include <utility>

Game::Game(const std::string& answerRepositoryFilepath, const std::string& questionRepositoryFilepath,
	 int numOfPlayers, int numOfAnswers, int numOfRounds) :
	m_answerRepository(AnswerRepository(answerRepositoryFilepath)),
	m_questionRepository(QuestionRepository(questionRepositoryFilepath)),
	m_numOfPlayers(numOfPlayers),
	m_numOfAnswers(numOfAnswers),
	m_numOfRounds(numOfRounds)
{
	srand(time(0));
	m_answers.resize(m_numOfPlayers);
	m_usedAnswers.resize(m_numOfPlayers);
	for (auto& answers : m_answers)
	{
		answers.resize(numOfAnswers);
	}
	for (auto& usedAnswers : m_usedAnswers)
	{
		usedAnswers.resize(numOfAnswers);
		std::fill(usedAnswers.begin(), usedAnswers.end(), true);
	}
}

void Game::generateData()
{
	generateTsarIndex();
	generateQuestion();
	for (int i = 0; i < m_numOfPlayers; i++)
	{
		if (i != m_tsarIndex)
		{
			generateAnswer(i);
		}
	}
}

void Game::generateTsarIndex()
{
	m_tsarIndex = 0;
	//m_tsarIndex = rand() % m_numOfPlayers;
}

void Game::generateQuestion()
{
	m_question = m_questionRepository.getQuestion();
}

void Game::generateAnswer(int playerIndex)
{
	for (int i = 0; i < m_numOfAnswers; i++)
	{
		if (m_usedAnswers[playerIndex][i])
		{
			m_answers[playerIndex][i] = std::pair<int, std::string>(i, m_answerRepository.getAnswer());
			m_usedAnswers[playerIndex][i] = false;
		}
	}
}