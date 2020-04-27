#include "Game.h"

#include <algorithm>
#include <ctime>
#include <utility>

Game::Game(std::unique_ptr<Repository<Prompt>> promptRepository,
		   std::unique_ptr<Repository<StatementCard>> statementCardRepository,
		   std::unique_ptr<GameDataManager> dataManager,
		   const GameConfiguration& configuration) :
	promptRepository{ std::move(promptRepository) },
	statementCardRepository{ std::move(statementCardRepository) },
	dataManager{ std::move(dataManager) },
	configuration{ configuration },
	state { GameState(configuration) }
{
	
}

void Game::generateRoundData()
{
	// generate a tsar index
	state.currentTsarIndex = dataManager->generatePlayerIndex(configuration.numOfPlayers);
	// generate a new prompt
	int promptIndex = dataManager->generateUniqueRepositoryIndex("prompt", promptRepository->size());
	state.currentPrompt = promptRepository->getObject(promptIndex);
	// generate new statement cards in the place of the ones that have been used for all players
	for (int playerIndex = 0; playerIndex < configuration.numOfPlayers; playerIndex++)
	{
		if (playerIndex != state.currentTsarIndex) // don't need new cards for tsar
		{
			for (int usedCardFlagIndex = 0; usedCardFlagIndex < state.usedStatementCards[playerIndex].size(); usedCardFlagIndex++)
			{
				if (state.usedStatementCards[playerIndex][usedCardFlagIndex])
				{
					int cardIndex = dataManager->generateUniqueRepositoryIndex("statementCard", statementCardRepository->size());
					state.usedStatementCards[playerIndex][usedCardFlagIndex] = false;
					state.statementCards[playerIndex][usedCardFlagIndex] = statementCardRepository->getObject(cardIndex).text;
				}
			}
		}
	}
}