#pragma once

#include "FileRepository.h"
#include "Prompt.h"
#include "StatementCard.h"
#include "GameDataManager.h"

#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

using PlayerIndex = int;
using StatementText = std::string;

class GameConfiguration
{
	public: 
		GameConfiguration() = default;
		GameConfiguration(int numOfPlayers,
						  int numOfRounds,
						  int numOfStatementCards):
			numOfPlayers{ numOfPlayers },
			numOfRounds{ numOfRounds },
			numOfStatementCards{ numOfStatementCards }
		{

		}
		
		int numOfPlayers;
		int numOfRounds;
		int numOfStatementCards;
};

class GameState
{
	public:
		GameState() = default;
		GameState(const GameConfiguration& configuration)
		{
			statementCards.resize(configuration.numOfPlayers);
			usedStatementCards.resize(configuration.numOfPlayers);
			for (auto& playerStatementCards : statementCards)
			{
				playerStatementCards.resize(configuration.numOfStatementCards);
			}
			for (auto& playerStatementCards : usedStatementCards)
			{
				playerStatementCards.resize(configuration.numOfStatementCards);
				std::fill(playerStatementCards.begin(), playerStatementCards.end(), true);
			}
		}

		int currentTsarIndex;
		Prompt currentPrompt;
		std::vector<std::vector<StatementText>> statementCards;
		std::vector<std::vector<bool>> usedStatementCards;
};

class Game
{
	public:
		Game(std::unique_ptr<Repository<Prompt>> promptRepository,
			 std::unique_ptr<Repository<StatementCard>> statementCardRepository,
			 std::unique_ptr<GameDataManager> dataManager,
			 const GameConfiguration& configuration);

		void generateRoundData();

		inline void setNumOfRounds(int numOfRounds) 
		{
			configuration.numOfRounds = numOfRounds;
		}

		inline void setPlayerStatementCardAsUsed(int playerIndex, int answerIndex) 
		{
			state.usedStatementCards[playerIndex][answerIndex] = true; 
		}

		inline const GameConfiguration& getGameConfiguration() 
		{
			return configuration;
		}

		inline const GameState& getGameState()
		{
			return state;
		}

		inline const int& getCurrentTsarIndex() const
		{
			return state.currentTsarIndex; 
		}

		inline const Prompt& getCurrentPrompt() const
		{
			return state.currentPrompt; 
		}

		inline const std::vector<StatementText>& getStatementCardsOfPlayer(int playerIndex) const
		{
			return state.statementCards[playerIndex]; 
		}
	private:
		std::unique_ptr<Repository<Prompt>> promptRepository;
		std::unique_ptr<Repository<StatementCard>> statementCardRepository;
		std::unique_ptr<GameDataManager> dataManager;
		GameConfiguration configuration;
		GameState state;
};