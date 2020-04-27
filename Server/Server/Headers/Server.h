#pragma once

#include "Client.h"
#include "CounterLock.h"
#include "FlagLock.h"
#include "Game.h"
#include "WNetwok.h"

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

class Interface;
class Game;

class Server
{
	public:
		Server(Interface& userInterface, const std::string& ip, short port, const std::string& settingsFilepath);
		~Server();

		void start();
	private:
		void loadSettings();

		void acceptConnections();
		void receiveUsernameFromClient(std::unique_ptr<Client>& client);
		bool validUsername(std::unique_ptr<Client>& client);

		std::vector<std::string> getPlayerList() const;
		void sendPlayerIDToClient(int clientIndex);
		void sendPlayerListToClient(int clientIndex);
		void sendNumOfRoundsToClient(int clientIndex);
		void sendNumOfStatementCardsToClient(int clientIndex);
		void sendGeneratedTsarIndexToClient(int clientIndex);
		void sendGeneratedPromptToClient(int clientIndex);
		void sendGeneratedStatementCardsToClient(int clientIndex);
		void sendStatementCardChoicesToClient(int clientIndex);
		void sendTsarStatementCardChoiceToClient(int clientIndex);
		void sendServerConfirmationToClient(int clientIndex);

		void receiveStatementCardChoiceFromClient(int clientIndex);
		void receiveStatementCardChoiceFromTsar(int clientIndex);
		void receiveNextRoundConfirmationFromClient(int clientIndex);

		void gameLogic();
		void sendClient(int clientIndex);
		void receiveClient(int clientIndex);
		void resetData();

		void generateData_();
		void shuffleStatementCards_();
		void resetData_();

		void sendData_(int clientIndex);
		void sendStatementCardChoices_(int clientIndex);
		void sendTsarChoice_(int clientIndex);
		void sendConfirmation_(int clientIndex);

		void receiveData_(int clientIndex);
		void receiveConfirmation_(int clientIndex);

		std::shared_ptr<WSAManager> wsaManager;
		Socket listening;

		std::string settingsFilepath;
		std::unique_ptr<Game> game;
		Interface& userInterface;

		std::vector<std::unique_ptr<Client>> clients;
		std::thread masterThread;
		std::vector<std::thread> sendClientThreads;
		std::vector<std::thread> receiveClientThreads;

		std::map<int, std::pair<int, std::vector<std::string>>> playerStatementCardChoices;
		int tsarChoiceIndex;

		bool doneGenerating;
		std::condition_variable doneGeneratingCvar;
		std::mutex doneGeneratingMutex;

		std::atomic<int> dataSentCounter;
		std::condition_variable dataSentCvar;
		std::mutex dataSentMutex;

		std::atomic<int> receivedStatementCardChoicesCounter;
		std::condition_variable receivedStatementCardChoicesCvar;
		std::mutex receivedStatementCardChoicesMutex;

		bool shuffledStatementCards;
		std::condition_variable shuffledStatementCardsCvar;
		std::mutex shuffledStatementCardsMutex;

		std::atomic<int> sentStatementCardChoicesCounter;
		std::condition_variable sentStatementCardChoicesCvar;
		std::mutex sentStatementCardChoicesMutex;

		bool receivedTsarStatementCard;
		std::condition_variable receivedTsarStatementCardCvar;
		std::mutex receivedTsarStatementCardMutex;

		std::atomic<int> sentTsarStatementCardChoiceCounter;
		std::condition_variable sentTsarStatementCardChoiceCvar;
		std::mutex sentTsarStatementCardChoiceMutex;

		std::atomic<int> sentServerConfirmationCounter;
		std::condition_variable sentServerConfirmationCvar;
		std::mutex sentServerConfirmationMutex;

		std::atomic<int> receivedNextRoundConfirmationCounter;
		std::condition_variable receivedNextRoundConfirmationCvar;
		std::mutex receivedNextRoundConfirmationMutex;

		bool resetFlags;
		std::condition_variable resetFlagsCvar;
		std::mutex resetFlagsMutex;
};