#pragma once

#include "WNetwork.h"

#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <condition_variable>
#include <mutex>

class Interface;

class Client
{
	using Score = int;

	public:
		Client(Interface& userInterface);
		~Client();

		inline void setUsername(const std::string& username) { this->username = username; }

		void connectToServer();
		void sendUsername();
		void receivePlayerList();
		void receiveNumOfRounds();
		void receiveNumOfAnswers();
		void start();
	private:
		void loadSettings();
		
		void receivePlayerID();
		void receiveTsarIndex();
		void receivePrompt();
		void receiveStatementCards();
		void receiveStatementCardChoices();
		void receiveTsarChoice();
		void receiveServerConfirmation();

		void sendChoice(const std::string& choice);
		void sendTsarChoice(const std::string& choice);

		void send();
		void receive();

		void sendChoice_();
		void sendTsarChoice_();
		void sendConfirmation_();
		void resetData_();

		void receiveData_(int i);
		void receiveStatementCardChoices_();
		void receiveTsarChoice_();
		void receiveConfirmation_();

		void displayInformation(int round);
		void displayChoices();
		void displayTsarChoice();
		void confirmNextRound();
		void resetData();

		std::shared_ptr<WSAManager> wsaManager;
		std::string username;
		std::string serverIP;
		unsigned short serverPort;

		const std::string settingsFilepath;

		Socket socket;

		Interface& userInterface;

		int playerID;
		int numOfRounds;
		std::vector<std::pair<Score, std::string>> playerList;

		std::thread sendThread;
		std::thread receiveThread;

		int tsarIndex;
		int promptNumOfBlanks;
		int tsarChoiceIndex;
		int winnerIndex;
		std::vector<std::vector<std::string>> statementCardChoices;
		std::string prompt;
		std::vector<std::string> statementCards;

		bool doneReceivingData;
		std::condition_variable dataReceived;
		std::mutex dataReceivedMutex;

		bool sentChoice;
		std::condition_variable choiceSent;
		std::mutex choiceSentMutex;

		bool receivedChoices;
		std::condition_variable choicesReceived;
		std::mutex choicesReceivedMutex;

		bool m_sentTsarChoice;
		std::condition_variable m_tsarChoiceSent;
		std::mutex m_tsarChoiceSentMutex;

		bool receivedTsarChoice;
		std::condition_variable tsarChoiceReceived;
		std::mutex tsarChoiceReceivedMutex;

		bool sentConfirmation;
		std::condition_variable confirmationSent;
		std::mutex confirmationSentMutex;

		bool receivedConfirmation;
		std::condition_variable confirmationReceived;
		std::mutex confirmationReceivedMutex;

		bool flagsReset;
		std::condition_variable resetFlags;
		std::mutex resetFlagsMutex;
};