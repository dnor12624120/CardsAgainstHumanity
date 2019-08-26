#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

#include "AnswerRepository.h"
#include "QuestionRepository.h"

class Game
{
	public:
		Game(const std::string& answerRepositoryFilepath, const std::string& questionRepositoryFilepath,
		 int numOfPlayers, int numOfAnswers, int numOfRounds);
		void generateData();

		inline void setNumOfRounds(int numOfRounds) { m_numOfRounds = numOfRounds; }
		inline void setUsedAnswer(int playerIndex, int answerIndex) { m_usedAnswers[playerIndex][answerIndex] = true; }
		inline const int getNumOfPlayers() const { return m_numOfPlayers; }
		inline const int getNumOfRounds() const { return m_numOfRounds; }
		inline const int getNumOfAnswers() const { return m_numOfAnswers; }

		inline const int& getTsarIndex() const { return m_tsarIndex; }
		inline const Question& getQuestion() const { return m_question; }
		inline const std::vector<std::pair<int, std::string>>& getAnswers(int playerIndex) const { return m_answers[playerIndex]; }
	private:
		void generateTsarIndex();
		void generateQuestion();
		void generateAnswer(int playerIndex);

		AnswerRepository m_answerRepository;
		QuestionRepository m_questionRepository;
		int m_numOfPlayers;
		int m_numOfRounds;
		int m_numOfAnswers;

		int m_tsarIndex;
		Question m_question;
		std::vector<std::vector<std::pair<int, std::string>>> m_answers;
		std::vector<std::vector<bool>> m_usedAnswers;
};