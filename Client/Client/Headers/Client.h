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

		inline void setUsername(const std::string& username) { m_username = username; }

		void connectToServer();
		void sendUsername();
		void receivePlayerList();
		void receiveNumOfRounds();
		void receiveNumOfAnswers();
		void start();
	private:
		void initNetwork();
		void loadSettings();
		
		void receivePlayerID();
		void receiveTsarIndex();
		void receiveQuestion();
		void receiveAnswers();
		void receiveAnswerChoices();
		void receiveTsarChoice();
		void receiveServerConfirmation();

		void sendChoice(const std::string& choice);
		void sendTsarChoice(const std::string& choice);

		void send();
		void receive();

		void displayInformation(int round);
		void displayChoices();
		void displayTsarChoice();
		void confirmNextRound();
		void resetFlags();

		std::shared_ptr<NetworkManager> m_networkManager;
		std::string m_username;
		std::string m_serverIP;
		unsigned short m_serverPort;

		const std::string m_settingsFilepath;

		Socket m_socket;

		Interface& m_userInterface;

		int m_playerID;
		int m_numOfRounds;
		std::vector<std::pair<Score, std::string>> m_playerList;

		std::thread m_sendThread;
		std::thread m_receiveThread;

		int m_tsarIndex;
		int m_questionNumOfBlanks;
		int m_tsarChoiceIndex;
		int m_winnerIndex;
		std::vector<std::vector<std::string>> m_answerChoices;
		std::string m_question;
		std::vector<std::string> m_answers;

		bool m_doneReceivingData;
		std::condition_variable m_dataReceived;
		std::mutex m_dataReceivedMutex;

		bool m_sentChoice;
		std::condition_variable m_choiceSent;
		std::mutex m_choiceSentMutex;

		bool m_receivedChoices;
		std::condition_variable m_choicesReceived;
		std::mutex m_choicesReceivedMutex;

		bool m_sentTsarChoice;
		std::condition_variable m_tsarChoiceSent;
		std::mutex m_tsarChoiceSentMutex;

		bool m_receivedTsarChoice;
		std::condition_variable m_tsarChoiceReceived;
		std::mutex m_tsarChoiceReceivedMutex;

		bool m_sentConfirmation;
		std::condition_variable m_confirmationSent;
		std::mutex m_confirmationSentMutex;

		bool m_receivedConfirmation;
		std::condition_variable m_confirmationReceived;
		std::mutex m_confirmationReceivedMutex;

		bool m_flagsReset;
		std::condition_variable m_resetFlags;
		std::mutex m_resetFlagsMutex;
};