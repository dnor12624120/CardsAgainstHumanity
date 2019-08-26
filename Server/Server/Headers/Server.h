#pragma once

#include "Client.h"
#include "Game.h"
#include "WNetwork.h"

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
		Server(Interface& userInterface);
		~Server();

		void start();
	private:
		void initNetwork();
		void loadSettings();

		void acceptConnections();
		void getUsername(std::unique_ptr<Client>& client);
		bool validUsername(std::unique_ptr<Client>& client);

		std::vector<std::string> getPlayerList() const;
		void sendPlayerID(int clientIndex);
		void sendPlayerList(int clientIndex);
		void sendNumOfRounds(int clientIndex);
		void sendNumOfAnswers(int clientIndex);
		void sendTsarIndex(int clientIndex);
		void sendQuestion(int clientIndex);
		void sendAnswers(int clientIndex);
		void sendAnswerChoices(int clientIndex);
		void sendTsarChoice(int clientIndex);
		void sendServerConfirmation(int clientIndex);

		void receiveAnswerChoice(int clientIndex);
		void receiveTsarChoice(int clientIndex);
		void receiveNextRoundConfirmation(int clientIndex);

		void gameLogic();
		void sendClient(int clientIndex);
		void receiveClient(int clientIndex);
		void resetFlags();

		std::shared_ptr<NetworkManager> m_networkManager;
		Socket m_listening;
		std::string m_serverIP;
		unsigned short m_serverPort;

		const std::string m_settingsFilepath;

		std::vector<std::unique_ptr<Client>> m_clients;
		std::vector<std::pair<int, std::vector<std::string>>> m_playerChoices;
		int m_tsarChoiceIndex;

		std::unique_ptr<Game> m_game;
		int m_tsarIndex;

		std::thread m_masterThread;
		std::vector<std::thread> m_sendClientThreads;
		std::vector<std::thread> m_receiveClientThreads;

		Interface& m_userInterface;

		bool m_doneGenerating;
		std::condition_variable m_dataGenerated;
		std::mutex m_dataGeneratedMutex;

		std::atomic<int> m_dataSentCounter;
		std::condition_variable m_dataSent;
		std::mutex m_dataSentMutex;

		std::atomic<int> m_receivedAnswerChoicesCounter;
		std::condition_variable m_receivedAnswerChoices;
		std::mutex m_receivedAnswerChoicesMutex;

		bool m_shuffledAnswers;
		std::condition_variable m_answersShuffled;
		std::mutex m_answersShuffledMutex;

		std::atomic<int> m_sentAnswerChoicesCounter;
		std::condition_variable m_sentAnswerChoices;
		std::mutex m_sentAnswerChoicesMutex;

		bool m_receivedTsarChoice;
		std::condition_variable m_tsarChoiceReceived;
		std::mutex m_tsarChoiceReceivedMutex;

		std::atomic<int> m_sentTsarChoiceCounter;
		std::condition_variable m_tsarChoiceSent;
		std::mutex m_tsarChoiceSentMutex;

		std::atomic<int> m_sentServerConfirmationCounter;
		std::condition_variable m_serverConfirmationSent;
		std::mutex m_serverConfirmationMutex;

		std::atomic<int> m_receivedNextRoundConfirmationCounter;
		std::condition_variable m_receivedNextRoundConfirmation;
		std::mutex m_receivedNextRoundConfirmationMutex;

		bool m_resetFlags;
		std::condition_variable m_flagsReset;
		std::mutex m_flagsResetMutex;
};