#include "Client.h"
#include "Interface.h"

#include <fstream>
#include <iostream>

Client::Client(Interface& userInterface):
	m_networkManager(NetworkManager::GetInstance()),
	m_socket(Socket(Family_IPv4, SocketType_Stream, Protocol_TCP)),
	m_userInterface(userInterface),
	m_settingsFilepath("settings.cfg"),
	m_doneReceivingData(false),
	m_sentChoice(false),
	m_receivedChoices(false),
	m_sentTsarChoice(false),
	m_receivedTsarChoice(false),
	m_sentConfirmation(false),
	m_receivedConfirmation(false),
	m_flagsReset(false)
{
	initNetwork();
	loadSettings();
}

Client::~Client()
{
	m_socket.Close();
	m_networkManager->Cleanup();
}

void Client::initNetwork()
{
	try
	{
		m_networkManager->Initialize(2, 2);
		m_socket.Create();
		m_socket.ToggleNagle(false);
		m_userInterface.notify("WNetwork initialized successfully!");
	}
	catch (WNException& exception)
	{
		m_userInterface.notify(exception.what());
	}
}

void Client::loadSettings()
{
	std::ifstream settingsFile(m_settingsFilepath);
	std::string settingType;
	std::string serverIP;
	unsigned short serverPort;

	settingsFile >> settingType >> m_serverIP;
	settingsFile >> settingType >> m_serverPort;

	settingsFile.close();
}

void Client::connectToServer()
{
	m_socket.Connect(IPv4Address(m_serverIP, m_serverPort));
	m_userInterface.notify("Connected to server! Waiting for other players to log in.");
}

void Client::sendUsername()
{
	int usernameLength = m_username.length();
	m_socket.SendAll(&usernameLength, sizeof(int));
	m_socket.SendAll(&m_username[0], usernameLength);
	bool validUsername = false;
	m_socket.RecieveAll(&validUsername, sizeof(bool));
	if (!validUsername)
	{
		throw std::exception("Invalid username!");
	}
}

void Client::receivePlayerList()
{
	int numOfPlayers;
	m_socket.RecieveAll(&numOfPlayers, sizeof(int));
	m_playerList.resize(numOfPlayers);
	for (auto& pair : m_playerList)
	{
		pair.first = 0;
	}
	for (int i = 0; i < numOfPlayers; i++)
	{
		int usernameLength;
		m_socket.RecieveAll(&usernameLength, sizeof(int));
		m_playerList[i].second.resize(usernameLength);
		m_socket.RecieveAll(&m_playerList[i].second[0], usernameLength);
	}
}

void Client::receiveNumOfRounds()
{
	m_socket.RecieveAll(&m_numOfRounds, sizeof(int));
}

void Client::receiveNumOfAnswers()
{
	int numOfAnswers;
	m_socket.RecieveAll(&numOfAnswers, sizeof(int));
	m_answers.resize(numOfAnswers);
}

void Client::start()
{
	sendUsername();
	receivePlayerID();
	receivePlayerList();
	receiveNumOfRounds();
	receiveNumOfAnswers();
	m_sendThread = std::thread(&Client::send, this);
	m_receiveThread = std::thread(&Client::receive, this);
	m_receiveThread.join();
	m_sendThread.join();
}

void Client::send()
{
	for (int i = 0; i < m_numOfRounds; i++)
	{
		std::unique_lock<std::mutex> dataReceivedLock(m_dataReceivedMutex);
		m_dataReceived.wait(dataReceivedLock, [this]{ return m_doneReceivingData; });
		if (m_playerID != m_tsarIndex)
		{
			std::string finalChoice; //TODO: move this to a separate function
			for (int j = 0; j < m_questionNumOfBlanks; j++)
			{
				bool validInput = false;
				std::vector<std::string> selectedAnswers;
				while (validInput == false)
				{
					std::string choice = m_userInterface.requestInput();
					if (choice.length() != 1 && !isdigit(choice[0]))
					{
						m_userInterface.notify("Enter a digit corresponding to the answer you wish to select.");
					}
					else if (choice[0] - '0' < 1 || choice[0] - '0' > m_answers.size())
					{
						m_userInterface.notify("Digit must be between 1-" + std::to_string(m_answers.size()));
					}
					else if (std::find(selectedAnswers.begin(), selectedAnswers.end(), choice) != selectedAnswers.end())
					{
						m_userInterface.notify("Answer already chosen.");
					}
					else
					{
						selectedAnswers.emplace_back(choice);
						finalChoice += choice;
						validInput = true;
					}
				}
			}
			sendChoice(finalChoice);
			m_sentChoice = true;
		}
		m_choiceSent.notify_all();
		if(m_playerID == m_tsarIndex)
		{
			std::unique_lock<std::mutex> choicesReceivedLock(m_choicesReceivedMutex);
			m_choicesReceived.wait(choicesReceivedLock, [this]{ return m_receivedChoices; });
			choicesReceivedLock.unlock();
			bool validInput = false;
			std::string	choice;
			while (validInput == false)
			{
				choice = m_userInterface.requestInput();
				if (choice.length() != 1 && !isdigit(choice[0]))
				{
					m_userInterface.notify("Enter a digit corresponding to the answer you wish to select.");
				}
				else if (choice[0] - '0' < 1 || choice[0] - '0' > m_playerList.size() - 1)
				{
					m_userInterface.notify("Digit must be between 1-" + std::to_string(m_answers.size()));
				}
				else
				{
					validInput = true;
				}
			}
			sendTsarChoice(choice);
			m_sentTsarChoice = true;
		}
		m_tsarChoiceSent.notify_all();
		std::unique_lock<std::mutex> tsarChoiceReceivedLock(m_tsarChoiceReceivedMutex);
		m_tsarChoiceReceived.wait(tsarChoiceReceivedLock, [this]{ return m_receivedTsarChoice; });
		tsarChoiceReceivedLock.unlock();
		confirmNextRound();
		m_sentConfirmation = true;
		m_confirmationSent.notify_all();
		std::unique_lock<std::mutex> receivedConfirmationLock(m_confirmationReceivedMutex);
		m_confirmationReceived.wait(receivedConfirmationLock, [this]{return m_receivedConfirmation; });
		receivedConfirmationLock.unlock();
		resetFlags();
		m_flagsReset = true;
		m_resetFlags.notify_all();
	}
}

void Client::receive()
{
	for (int i = 0; i < m_numOfRounds; i++)
	{
		receiveTsarIndex();
		receiveQuestion();
		if (m_playerID != m_tsarIndex)
		{
			receiveAnswers();
		}
		m_doneReceivingData = true;
		displayInformation(i);
		m_dataReceived.notify_all();
		std::unique_lock<std::mutex> choiceSentLock(m_choiceSentMutex);
		m_choiceSent.wait(choiceSentLock, [this]{ return m_sentChoice || m_playerID == m_tsarIndex; });
		choiceSentLock.unlock();
		receiveAnswerChoices();
		displayChoices();
		m_receivedChoices = true;
		m_choicesReceived.notify_all();
		std::unique_lock<std::mutex> tsarChoiceSentLock(m_tsarChoiceSentMutex);
		m_tsarChoiceSent.wait(tsarChoiceSentLock, [this]{ return m_sentTsarChoice || m_playerID != m_tsarIndex; });
		tsarChoiceSentLock.unlock();
		receiveTsarChoice();
		displayTsarChoice();
		m_receivedTsarChoice = true;
		m_tsarChoiceReceived.notify_all();
		std::unique_lock<std::mutex> confirmationSentLock(m_confirmationSentMutex);
		m_confirmationSent.wait(confirmationSentLock, [this]{ return m_sentConfirmation; });
		confirmationSentLock.unlock();
		receiveServerConfirmation();
		m_receivedConfirmation = true;
		m_confirmationReceived.notify_all();
		std::unique_lock<std::mutex> resetFlagsLock(m_resetFlagsMutex);
		m_resetFlags.wait(resetFlagsLock, [this]{ return m_flagsReset; });
		resetFlagsLock.unlock();
	}
}

void Client::sendChoice(const std::string& choice)
{
	for (int i = 0; i < m_questionNumOfBlanks; i++)
	{
		int choiceInt = choice[i] - '0' - 1;
		m_socket.SendAll(&choiceInt, sizeof(int));
		int answerLength = m_answers[choiceInt].length();
		m_socket.SendAll(&answerLength, sizeof(int));
		m_socket.SendAll(&m_answers[choiceInt][0], answerLength);
	}
}

void Client::sendTsarChoice(const std::string& choice)
{
	int choiceInt = atoi(choice.c_str()) - 1;
	m_socket.SendAll(&choiceInt, sizeof(int));
}

void Client::receivePlayerID()
{
	m_socket.RecieveAll(&m_playerID, sizeof(int));
}

void Client::receiveTsarIndex()
{
	m_socket.RecieveAll(&m_tsarIndex, sizeof(int));
}

void Client::receiveQuestion()
{
	int questionLength;
	m_socket.RecieveAll(&questionLength, sizeof(int));
	m_question.resize(questionLength);
	m_socket.RecieveAll(&m_question[0], questionLength);
	m_socket.RecieveAll(&m_questionNumOfBlanks, sizeof(int));
}

void Client::receiveAnswers()
{
	int numOfAnswers;
	m_socket.RecieveAll(&numOfAnswers, sizeof(int));
	for (int i = 0; i < numOfAnswers; i++)
	{
		int questionIndex;
		m_socket.RecieveAll(&questionIndex, sizeof(int));
		int questionLength;
		m_socket.RecieveAll(&questionLength, sizeof(int));
		std::string question;
		question.resize(questionLength);
		m_socket.RecieveAll(&question[0], questionLength);
		m_answers[questionIndex] = question;
	}
}

void Client::receiveAnswerChoices()
{
	int numOfAnswers;
	m_socket.RecieveAll(&numOfAnswers, sizeof(int));
	for (int i = 0; i < numOfAnswers; i++)
	{
		std::vector<std::string> answers;
		for (int j = 0; j < m_questionNumOfBlanks; j++)
		{
			int answerLength;
			std::string answer;
			m_socket.RecieveAll(&answerLength, sizeof(int));
			answer.resize(answerLength);
			m_socket.RecieveAll(&answer[0], answerLength);
			answers.emplace_back(answer);
		}
		m_answerChoices.emplace_back(answers);
	}
}

void Client::receiveTsarChoice()
{
	m_socket.RecieveAll(&m_winnerIndex, sizeof(int));
	m_playerList[m_winnerIndex].first++;
	m_socket.RecieveAll(&m_tsarChoiceIndex, sizeof(int));
}

void Client::receiveServerConfirmation()
{
	bool confirmation;
	m_socket.RecieveAll(&confirmation, sizeof(bool));
}

void Client::displayInformation(int round)
{
	system("cls");
	m_userInterface.notify("Round #" + std::to_string(round + 1));
	m_userInterface.notify("Player: " + m_playerList[m_playerID].second);
	m_userInterface.notify("Player list:");
	for (int i = 0; i < m_playerList.size(); i++)
	{
		if (i == m_tsarIndex)
		{
			m_userInterface.notify(m_playerList[i].second + " - " + std::to_string(m_playerList[i].first) + " - Tsar");
		}
		else
		{
			m_userInterface.notify(m_playerList[i].second + " - " + std::to_string(m_playerList[i].first));
		}
	}
	m_userInterface.notify(m_question + "\n");
	if (m_playerID != m_tsarIndex)
	{
		for (int i = 0; i < m_answers.size(); i++)
		{
			m_userInterface.notify(std::to_string(i + 1) + ". " + m_answers[i]);
		}
	}
}

void Client::displayChoices()
{
	m_userInterface.notify("Player choices:\n");
	for (auto& choice : m_answerChoices)
	{
		std::string completeAnswer;
		for (int i = 0; i < choice.size() - 1; i++)
		{
			completeAnswer += choice[i] + " / ";
		}
		completeAnswer += choice[choice.size() - 1];
		m_userInterface.notify(completeAnswer);
	}
}

void Client::displayTsarChoice()
{
	m_userInterface.notify("Tsar choice:\n");
	std::string completeAnswer;
	for (int i = 0; i < m_answerChoices[m_tsarChoiceIndex].size() - 1; i++)
	{
		completeAnswer += m_answerChoices[m_tsarChoiceIndex][i] + " / ";
	}
	completeAnswer += m_answerChoices[m_tsarChoiceIndex][m_answerChoices[m_tsarChoiceIndex].size() - 1];
	m_userInterface.notify(completeAnswer);
}

void Client::confirmNextRound()
{
	m_userInterface.notify("Press any key to continue:");
	while (m_userInterface.requestInput().length() == 0);
	bool confirmed;
	m_socket.SendAll(&confirmed, sizeof(bool));
}

void Client::resetFlags()
{
	m_doneReceivingData = false;
	m_sentChoice = false;
	m_receivedChoices = false;
	m_sentTsarChoice = false;
	m_receivedTsarChoice = false;
	m_flagsReset = false;
	m_sentConfirmation = false;
	m_receivedConfirmation = false;
	m_answerChoices.clear();
}