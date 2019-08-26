#include "Interface.h"
#include "Server.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

Server::Server(Interface& userInterface):
	m_networkManager(NetworkManager::GetInstance()),
	m_listening(Socket(Family_IPv4, SocketType_Stream, Protocol_TCP)),
	m_settingsFilepath("settings.cfg"),
	m_userInterface(userInterface),
	m_doneGenerating(false),
	m_dataSentCounter(0),
	m_receivedAnswerChoicesCounter(0),
	m_shuffledAnswers(false),
	m_sentTsarChoiceCounter(0),
	m_sentServerConfirmationCounter(0),
	m_resetFlags(false)
{
	initNetwork();
	loadSettings();
	m_listening.Bind(IPv4Address(m_serverIP, m_serverPort));
}

Server::~Server()
{
	
}

void Server::initNetwork()
{
	try
	{
		m_networkManager->Initialize(2, 2);
		m_listening.Create();
		m_userInterface.notify("WNetwork initialized successfully!");
	}
	catch (WNException& exception)
	{
		m_userInterface.notify(exception.what());
	}
}

void Server::loadSettings()
{
	std::ifstream settingsFile(m_settingsFilepath);
	std::string settingType;
	std::string answerRepoFilepath;
	std::string questionRepoFilepath;
	int numOfPlayers;
	int numOfRounds;
	int numOfAnswers;

	settingsFile >> settingType >> m_serverIP;
	settingsFile >> settingType >> m_serverPort;
	settingsFile >> settingType >> numOfPlayers;
	settingsFile >> settingType >> numOfRounds;
	settingsFile >> settingType >> numOfAnswers;
	settingsFile >> settingType >> answerRepoFilepath;
	settingsFile >> settingType >> questionRepoFilepath;

	m_userInterface.notify("IP loaded from " + m_settingsFilepath + ": " + m_serverIP);
	m_userInterface.notify("Port loaded from " + m_settingsFilepath + ": " + std::to_string(m_serverPort));
	m_userInterface.notify("Game initialized with " + std::to_string(numOfPlayers) + " players, playing for " + std::to_string(numOfRounds) + " rounds");
	m_userInterface.notify("Read answers from " + answerRepoFilepath);
	m_userInterface.notify("Read questions from " + questionRepoFilepath);

	m_game = std::make_unique<Game>(answerRepoFilepath, questionRepoFilepath, numOfPlayers, numOfAnswers, numOfRounds);
}

void Server::start()
{
	m_listening.Listen();
	acceptConnections();
	m_userInterface.notify("All players connected. Starting game.");
	for (int i = 0; i < m_clients.size(); i++)
	{
		sendPlayerID(i);
		sendPlayerList(i);
		sendNumOfRounds(i);
		sendNumOfAnswers(i);
	}
	m_masterThread = std::thread(&Server::gameLogic, this);
	m_sendClientThreads.resize(m_clients.size());
	m_receiveClientThreads.resize(m_clients.size());
	for (int i = 0; i < m_clients.size(); i++)
	{
		m_sendClientThreads[i] = std::thread(&Server::sendClient, this, i);
		m_receiveClientThreads[i] = std::thread(&Server::receiveClient, this, i);
	}
	for (int i = 0; i < m_clients.size(); i++)
	{
		m_sendClientThreads[i].join();
		m_receiveClientThreads[i].join();
	}
	m_masterThread.join();
	auto winner = std::max_element(m_clients.begin(), m_clients.end(), [](auto& client1, auto& client2){ return client1->getScore() < client2->getScore(); });
	m_userInterface.notify(winner->get()->getUsername());
}

void Server::gameLogic()
{
	for (int i = 0; i < m_game->getNumOfRounds(); i++)
	{
		m_userInterface.notify("Round #" + std::to_string(i + 1));
		m_userInterface.notify("Tsar Index: " + std::to_string(m_tsarIndex));
		m_game->generateData();
		m_tsarIndex = m_game->getTsarIndex();
		m_doneGenerating = true;
		m_dataGenerated.notify_all();
		std::unique_lock<std::mutex> receivedAnswerChoicesLock(m_receivedAnswerChoicesMutex);
		m_resetFlags = false;
		m_receivedAnswerChoices.wait(receivedAnswerChoicesLock, [this]{ return m_receivedAnswerChoicesCounter == m_clients.size() - 1; });
		receivedAnswerChoicesLock.unlock();
		std::random_shuffle(m_playerChoices.begin(), m_playerChoices.end(), [](int i){ return rand() % i; });
		m_shuffledAnswers = true;
		m_answersShuffled.notify_all();
		std::unique_lock<std::mutex> serverConfirmationLock(m_serverConfirmationMutex);
		m_serverConfirmationSent.wait(serverConfirmationLock, [this]{ return m_sentServerConfirmationCounter == m_clients.size(); });
		serverConfirmationLock.unlock();
		resetFlags();
		m_resetFlags = true;
		m_flagsReset.notify_all();
	}
}

void Server::sendClient(int clientIndex)
{
	for (int i = 0; i < m_game->getNumOfRounds(); i++)
	{
		std::unique_lock<std::mutex> dataGeneratedLock(m_dataGeneratedMutex);
		m_dataGenerated.wait(dataGeneratedLock, [this]{ return m_doneGenerating; });
		dataGeneratedLock.unlock();
		m_userInterface.notify("Sending data to " + m_clients[clientIndex]->getUsername());
		sendTsarIndex(clientIndex);
		sendQuestion(clientIndex);
		if (clientIndex != m_tsarIndex)
		{
			sendAnswers(clientIndex);
		}
		m_dataSentCounter++;
		m_dataSent.notify_all();
		std::unique_lock<std::mutex> answersShuffledLock(m_answersShuffledMutex);
		m_answersShuffled.wait(answersShuffledLock, [this]{ return m_shuffledAnswers; });
		answersShuffledLock.unlock();
		sendAnswerChoices(clientIndex);
		m_userInterface.notify("Sent player choices to " + m_clients[clientIndex]->getUsername());
		m_sentAnswerChoicesCounter++;
		m_sentAnswerChoices.notify_all();
		std::unique_lock<std::mutex> tsarChoiceReceivedLock(m_tsarChoiceReceivedMutex);
		m_tsarChoiceReceived.wait(tsarChoiceReceivedLock, [this]{ return m_receivedTsarChoice; });
		tsarChoiceReceivedLock.unlock();
		sendTsarChoice(clientIndex);
		m_userInterface.notify("Sent tsar choice to " + m_clients[clientIndex]->getUsername());
		m_sentTsarChoiceCounter++;
		m_tsarChoiceSent.notify_all();
		std::unique_lock<std::mutex> receivedNextRoundConfirmationLock(m_receivedNextRoundConfirmationMutex);
		m_receivedNextRoundConfirmation.wait(receivedNextRoundConfirmationLock, [this]{ return m_receivedNextRoundConfirmationCounter == m_clients.size(); });
		receivedNextRoundConfirmationLock.unlock();
		sendServerConfirmation(clientIndex);
		m_sentServerConfirmationCounter++;
		m_serverConfirmationSent.notify_all();
		std::unique_lock<std::mutex> flagsResetLock(m_flagsResetMutex);
		m_flagsReset.wait(flagsResetLock, [this]{ return m_resetFlags; });
		flagsResetLock.unlock();
	}
}

void Server::receiveClient(int clientIndex)
{
	for (int i = 0; i < m_game->getNumOfRounds(); i++)
	{
		std::unique_lock<std::mutex> dataSentLock(m_dataSentMutex);
		m_dataSent.wait(dataSentLock, [this]{ return m_dataSentCounter == m_clients.size(); });
		dataSentLock.unlock();
		if (clientIndex != m_tsarIndex)
		{
			receiveAnswerChoice(clientIndex);
			m_userInterface.notify("Received answer from " + m_clients[clientIndex]->getUsername());
			m_receivedAnswerChoicesCounter++;
			m_receivedAnswerChoices.notify_all();
		}
		else
		{
			std::unique_lock<std::mutex> sentAnswerChoicesLock(m_sentAnswerChoicesMutex);
			m_sentAnswerChoices.wait(sentAnswerChoicesLock, [this]{ return m_sentAnswerChoicesCounter == m_clients.size(); });
			sentAnswerChoicesLock.unlock();
			receiveTsarChoice(clientIndex);
			m_userInterface.notify("Received answer from tsar. - " + m_clients[clientIndex]->getUsername());
			m_receivedTsarChoice = true;
			m_tsarChoiceReceived.notify_all();
		}
		std::unique_lock<std::mutex> tsarChoiceSentLock(m_tsarChoiceSentMutex);
		m_tsarChoiceSent.wait(tsarChoiceSentLock, [this]{ return m_sentTsarChoiceCounter == m_clients.size(); });
		tsarChoiceSentLock.unlock();
		receiveNextRoundConfirmation(clientIndex);
		m_userInterface.notify("Received next round confirmation from " + m_clients[clientIndex]->getUsername());
		m_receivedNextRoundConfirmationCounter++;
		m_receivedNextRoundConfirmation.notify_all();
		std::unique_lock<std::mutex> flagsResetLock(m_flagsResetMutex);
		m_flagsReset.wait(flagsResetLock, [this]{ return m_resetFlags; });
		flagsResetLock.unlock();
	}
}

void Server::receiveAnswerChoice(int clientIndex)
{
	std::vector<std::string> answers;
	for (int i = 0; i < m_game->getQuestion().m_numOfBlanks; i++)
	{
		int choiceIndex;
		Socket socket = m_clients[clientIndex]->getSocket();
		socket.RecieveAll(&choiceIndex, sizeof(int));
		m_game->setUsedAnswer(clientIndex, choiceIndex);
		int answerLength;
		std::string answer;
		socket.RecieveAll(&answerLength, sizeof(int));
		answer.resize(answerLength);
		socket.RecieveAll(&answer[0], answerLength);
		answers.emplace_back(answer);
	}
	m_playerChoices.emplace_back(std::pair<int, std::vector<std::string>>(clientIndex, answers));
}

void Server::receiveTsarChoice(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	socket.RecieveAll(&m_tsarChoiceIndex, sizeof(int));
	m_clients[m_playerChoices[m_tsarChoiceIndex].first]->incrementScore();
}

void Server::receiveNextRoundConfirmation(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	bool ready;
	socket.RecieveAll(&ready, sizeof(bool));
}

void Server::acceptConnections()
{
	while (m_clients.size() < m_game->getNumOfPlayers())
	{
		try
		{
			FileDescriptorSet set;
			set.Clear();
			set.AddSocket(m_listening);
			set.SetTimeout(1, 0);
			if (set.Select() > 0)
			{
				std::unique_ptr<Client> client(std::make_unique<Client>());
				m_listening.Accept(client->getSocket(), client->getAddress());
				getUsername(client);
				if (validUsername(client))
				{
					m_userInterface.notify(client->getUsername() + " joined!");
					m_clients.emplace_back(std::move(client));
				}
			}
		}
		catch (WNException& exception)
		{
			m_userInterface.notify(exception.what());
		}
	}
}

void Server::getUsername(std::unique_ptr<Client>& client)
{
	int usernameLength;
	std::string username;
	client->getSocket().RecieveAll(&usernameLength, sizeof(int));
	username.resize(usernameLength);
	client->getSocket().RecieveAll(&username[0], usernameLength);
	client->setUsername(username);
}

bool Server::validUsername(std::unique_ptr<Client>& client)
{
	auto socket = client->getSocket();
	std::string username = client->getUsername();
	bool valid = true;
	for (int i = 0; i < m_clients.size() && valid; i++)
	{
		std::string clientUsername = m_clients[i]->getUsername();
		if (clientUsername == username)
		{
			valid = false;
		}
	}
	socket.SendAll(&valid, sizeof(bool));
	return valid;
}

std::vector<std::string> Server::getPlayerList() const
{
	std::vector<std::string> playerList;
	for (auto& client : m_clients)
	{
		playerList.emplace_back(client->getUsername());
	}
	return playerList;
}

void Server::sendPlayerID(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	socket.SendAll(&clientIndex, sizeof(int));
}

void Server::sendPlayerList(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	int numOfPlayers = m_clients.size();
	socket.SendAll(&numOfPlayers, sizeof(int));
	for (int i = 0; i < numOfPlayers; i++)
	{
		std::string username = m_clients[i]->getUsername();
		int usernameLength = username.length();
		socket.SendAll(&usernameLength, sizeof(int));
		socket.SendAll(&username[0], usernameLength);
	}
}

void Server::sendNumOfRounds(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	int numOfRounds = m_game->getNumOfRounds();
	socket.SendAll(&numOfRounds, sizeof(int));
}

void Server::sendNumOfAnswers(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	int numOfAnswers = m_game->getNumOfAnswers();
	socket.SendAll(&numOfAnswers, sizeof(int));
}

void Server::sendTsarIndex(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	socket.SendAll(&m_tsarIndex, sizeof(int));
}

void Server::sendQuestion(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	int questionLength = m_game->getQuestion().m_text.length();
	socket.SendAll(&questionLength, sizeof(int));
	socket.SendAll(&m_game->getQuestion().m_text[0], questionLength);
	socket.SendAll(&m_game->getQuestion().m_numOfBlanks, sizeof(int));
}

void Server::sendAnswers(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	auto answers = m_game->getAnswers(clientIndex);
	int numOfAnswers = answers.size();
	socket.SendAll(&numOfAnswers, sizeof(int));
	for (int i = 0; i < answers.size(); i++)
	{
		int answerIndex = answers[i].first;
		socket.SendAll(&answerIndex, sizeof(int));
		int answerLength = answers[i].second.length();
		socket.SendAll(&answerLength, sizeof(int));
		socket.SendAll(&answers[i].second[0], answerLength);
	}
}

void Server::sendAnswerChoices(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	int numOfAnswers = m_clients.size() - 1;
	socket.SendAll(&numOfAnswers, sizeof(int));
	for (int i = 0; i < numOfAnswers; i++)
	{
		for (int j = 0; j < m_game->getQuestion().m_numOfBlanks; j++)
		{
			try
			{
				int answerLength = m_playerChoices[i].second[j].length();
				socket.SendAll(&answerLength, sizeof(int));
				socket.SendAll(&m_playerChoices[i].second[j][0], answerLength);
			}
			catch (WNException& exception)
			{
				m_userInterface.notify(exception.what());
			}
		}
	}
}

void Server::sendTsarChoice(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	socket.SendAll(&m_playerChoices[m_tsarChoiceIndex].first, sizeof(int));
	socket.SendAll(&m_tsarChoiceIndex, sizeof(int));
}

void Server::sendServerConfirmation(int clientIndex)
{
	Socket socket = m_clients[clientIndex]->getSocket();
	bool confirmation;
	socket.SendAll(&confirmation, sizeof(bool));
}

void Server::resetFlags()
{
	m_doneGenerating = false;
	m_dataSentCounter = 0;
	m_receivedAnswerChoicesCounter = 0;
	m_shuffledAnswers = false;
	m_sentAnswerChoicesCounter = 0;
	m_receivedTsarChoice = false;
	m_sentTsarChoiceCounter = 0;
	m_sentServerConfirmationCounter = 0;
	m_receivedNextRoundConfirmationCounter = 0;
	m_resetFlags = false;
	m_playerChoices.clear();
}