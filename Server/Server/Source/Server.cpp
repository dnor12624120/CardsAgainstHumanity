#include "Interface.h"
#include "Server.h"
#include "FileRepository.h"
#include "GameDataManager.h"
#include "StdRandGenerator.h"
#include "Repository.h"
#include "WNetwok.h"
#include "StatementCard.h"
#include "GeneratorStrategy.h"
#include "Prompt.h"
#include "Game.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>

Server::Server(Interface& userInterface, const std::string& ip, short port, const std::string& settingsFilepath):
	wsaManager{ WSAManager::GetInstance() },
	listening{ Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) },
	settingsFilepath{ settingsFilepath },
	userInterface{ userInterface },
	doneGenerating{ false },
	dataSentCounter{ 0 },
	receivedStatementCardChoicesCounter{ 0 },
	shuffledStatementCards{ false },
	sentStatementCardChoicesCounter{ 0 },
	receivedTsarStatementCard{ false },
	sentTsarStatementCardChoiceCounter{ 0 },
	sentServerConfirmationCounter{ 0 },
	receivedNextRoundConfirmationCounter{ 0 },
	resetFlags{ false }
{
	loadSettings();
	listening.Bind(IPv4Address(ip, port));
}

Server::~Server()
{
	
}

void Server::loadSettings()
{
	std::ifstream settingsFile(settingsFilepath);
	std::string settingType;
	std::string statementCardRepoFilepath;
	std::string promptRepoFilepath;
	int numOfPlayers;
	int numOfRounds;
	int numOfStatementCards;

	settingsFile >> settingType >> numOfPlayers;
	settingsFile >> settingType >> numOfRounds;
	settingsFile >> settingType >> numOfStatementCards;
	settingsFile >> settingType >> statementCardRepoFilepath;
	settingsFile >> settingType >> promptRepoFilepath;

	userInterface.printMessage("Game initialized with " + std::to_string(numOfPlayers) + " players, playing for " + std::to_string(numOfRounds) + " rounds");
	userInterface.printMessage("Read answers from " + statementCardRepoFilepath);
	userInterface.printMessage("Read questions from " + promptRepoFilepath);

	std::unique_ptr<Repository<Prompt>> promptRepository = std::unique_ptr<Repository<Prompt>>(new FileRepository<Prompt>(promptRepoFilepath));
	std::unique_ptr<Repository<StatementCard>> statementCardRepository =
		std::unique_ptr<Repository<StatementCard>>(new FileRepository<StatementCard>(statementCardRepoFilepath));
	std::unique_ptr<GeneratorStrategy> strategy = std::unique_ptr<StdRandGenerator>(new StdRandGenerator());
	std::unique_ptr<GameDataManager> manager = std::unique_ptr<GameDataManager>(new GameDataManager(std::move(strategy)));
	GameConfiguration configuration(numOfPlayers, numOfRounds, numOfStatementCards);

	game = std::unique_ptr<Game>(new Game(std::move(promptRepository), std::move(statementCardRepository),
										  std::move(manager), configuration));
}

void Server::start()
{
	listening.Listen();
	acceptConnections();
	userInterface.printMessage("All players connected. Starting game.");
	for (int i = 0; i < clients.size(); i++)
	{
		sendPlayerIDToClient(i);
		sendPlayerListToClient(i);
		sendNumOfRoundsToClient(i);
		sendNumOfStatementCardsToClient(i);
	}
	masterThread = std::thread(&Server::gameLogic, this);
	sendClientThreads.resize(clients.size());
	receiveClientThreads.resize(clients.size());
	for (int i = 0; i < clients.size(); i++)
	{
		sendClientThreads[i] = std::thread(&Server::sendClient, this, i);
		receiveClientThreads[i] = std::thread(&Server::receiveClient, this, i);
	}
	for (int i = 0; i < clients.size(); i++)
	{
		sendClientThreads[i].join();
		receiveClientThreads[i].join();
	}
	masterThread.join();
	auto winner = std::max_element(clients.begin(), clients.end(), [](auto& client1, auto& client2){ return client1->getScore() < client2->getScore(); });
	userInterface.printMessage(winner->get()->getUsername());
}

void Server::acceptConnections()
{
	while (clients.size() < game->getGameConfiguration().numOfPlayers)
	{
		try
		{
			FileDescriptorSet set;
			set.Zero();
			set.AddSocket(listening);
			set.SetTimeout(1, 0);
			if (set.Select() > 0)
			{
				std::unique_ptr<Client> client(std::make_unique<Client>());
				listening.Accept(client->getSocket(), client->getAddress());
				receiveUsernameFromClient(client);
				if (validUsername(client))
				{
					userInterface.printMessage(client->getUsername() + " joined!");
					clients.emplace_back(std::move(client));
				}
			}
		}
		catch (WinSockException& exception)
		{
			userInterface.printMessage(exception.what());
		}
	}
}

void Server::gameLogic()
{
	srand(time(0));
	for (int i = 0; i < game->getGameConfiguration().numOfRounds; i++)
	{
		generateData_();
		shuffleStatementCards_();
		resetData_();
	}
}

void Server::sendClient(int clientIndex)
{
	for (int i = 0; i < game->getGameConfiguration().numOfRounds; i++)
	{
		sendData_(clientIndex);
		sendStatementCardChoices_(clientIndex);
		sendTsarChoice_(clientIndex);
		sendConfirmation_(clientIndex);
	}
}

void Server::receiveClient(int clientIndex)
{
	for (int i = 0; i < game->getGameConfiguration().numOfRounds; i++)
	{
		receiveData_(clientIndex);
		receiveConfirmation_(clientIndex);
	}
}

void Server::receiveStatementCardChoiceFromClient(int clientIndex)
{
	int choiceIndex;
	int statementCardTextLength;
	std::string statementCard;
	std::vector<std::string> statementCards;
	for (int i = 0; i < game->getGameState().currentPrompt.numOfBlanks; i++)
	{
		Socket socket = clients[clientIndex]->getSocket();
		socket.Receive(&choiceIndex, sizeof(int));
		game->setPlayerStatementCardAsUsed(clientIndex, choiceIndex);
		socket.Receive(&statementCardTextLength, sizeof(int));
		statementCard.resize(statementCardTextLength);
		socket.Receive(&statementCard[0], statementCardTextLength);
		statementCards.emplace_back(statementCard);
	}
	playerStatementCardChoices[clientIndex] = std::pair<int, std::vector<std::string>>(clientIndex, statementCards);
}

void Server::receiveStatementCardChoiceFromTsar(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	socket.Receive(&tsarChoiceIndex, sizeof(int));
	clients[playerStatementCardChoices[tsarChoiceIndex].first]->incrementScore();
}

void Server::receiveNextRoundConfirmationFromClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	bool ready; 
	socket.Receive(&ready, sizeof(bool));
	// we don't care what the bool is set to, just that its received 
}

void Server::receiveUsernameFromClient(std::unique_ptr<Client>& client)
{
	int usernameLength;
	std::string username;
	client->getSocket().Receive(&usernameLength, sizeof(int));
	username.resize(usernameLength);
	client->getSocket().Receive(&username[0], usernameLength);
	client->setUsername(username);
}

bool Server::validUsername(std::unique_ptr<Client>& client)
{
	auto socket = client->getSocket();
	std::string username = client->getUsername();
	bool valid = true;
	for (int i = 0; i < clients.size() && valid; i++)
	{
		std::string clientUsername = clients[i]->getUsername();
		if (clientUsername == username)
		{
			valid = false;
		}
	}
	socket.Send(&valid, sizeof(bool));
	return valid;
}

std::vector<std::string> Server::getPlayerList() const
{
	std::vector<std::string> playerList;
	for (auto& client : clients)
	{
		playerList.emplace_back(client->getUsername());
	}
	return playerList;
}

void Server::sendPlayerIDToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	socket.Send(&clientIndex, sizeof(int));
}

void Server::sendPlayerListToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	int numOfPlayers = clients.size();
	socket.Send(&numOfPlayers, sizeof(int));
	for (int i = 0; i < numOfPlayers; i++)
	{
		std::string username = clients[i]->getUsername();
		int usernameLength = username.length();
		socket.Send(&usernameLength, sizeof(int));
		socket.Send(&username[0], usernameLength);
	}
}

void Server::sendNumOfRoundsToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	int numOfRounds = game->getGameConfiguration().numOfRounds;
	socket.Send(&numOfRounds, sizeof(int));
}

void Server::sendNumOfStatementCardsToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	int numOfStatementCards = game->getGameConfiguration().numOfStatementCards;
	socket.Send(&numOfStatementCards, sizeof(int));
}

void Server::sendGeneratedTsarIndexToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	socket.Send(&(game->getGameState().currentTsarIndex), sizeof(int));
}

void Server::sendGeneratedPromptToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	std::string prompt = game->getGameState().currentPrompt.text;
	int promptNumOfBlanks = game->getGameState().currentPrompt.numOfBlanks;
	int promptTextLength = prompt.length();
	socket.Send(&promptTextLength, sizeof(int));
	socket.Send(&prompt[0], promptTextLength);
	socket.Send(&promptNumOfBlanks, sizeof(int));
}

void Server::sendGeneratedStatementCardsToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	auto statementCards = game->getGameState().statementCards[clientIndex];
	int numOfStatementCards = statementCards.size();
	socket.Send(&numOfStatementCards, sizeof(int));
	for (int i = 0; i < statementCards.size(); i++)
	{
		int statementCardLength = statementCards[i].length();
		socket.Send(&statementCardLength, sizeof(int));
		socket.Send(&statementCards[i][0], statementCardLength);
	}
}

void Server::sendStatementCardChoicesToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	int numOfChoices = clients.size() - 1; // tsar doesn't choose
	socket.Send(&numOfChoices, sizeof(int));
	for (int i = 0; i < clients.size(); i++)
	{
		if (i != game->getGameState().currentTsarIndex)
		{
			for (int j = 0; j < game->getGameState().currentPrompt.numOfBlanks; j++)
			{
				try
				{
					int statementCardTextLength = playerStatementCardChoices[i].second[j].length();
					socket.Send(&statementCardTextLength, sizeof(int));
					socket.Send(&playerStatementCardChoices[i].second[j][0], statementCardTextLength);
				}
				catch (WinSockException& exception)
				{
					userInterface.printMessage(exception.what());
				}
			}
		}
	}
}

void Server::sendTsarStatementCardChoiceToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	socket.Send(&playerStatementCardChoices[tsarChoiceIndex].first, sizeof(int));
	socket.Send(&tsarChoiceIndex, sizeof(int));
}

void Server::sendServerConfirmationToClient(int clientIndex)
{
	Socket socket = clients[clientIndex]->getSocket();
	bool confirmation;
	socket.Send(&confirmation, sizeof(bool));
}

void Server::resetData()
{
	doneGenerating = false;
	dataSentCounter = 0;
	receivedStatementCardChoicesCounter = 0;
	shuffledStatementCards = false;
	sentStatementCardChoicesCounter = 0;
	receivedTsarStatementCard = false;
	sentTsarStatementCardChoiceCounter = 0;
	sentServerConfirmationCounter = 0;
	receivedNextRoundConfirmationCounter = 0;
	resetFlags = false;
	playerStatementCardChoices.clear();
}

void Server::generateData_()
{
	game->generateRoundData();
	int tsarIndex = game->getGameState().currentTsarIndex;
	userInterface.printMessage("Tsar Index: " + std::to_string(tsarIndex));
	doneGenerating = true;
	doneGeneratingCvar.notify_all();
}

void Server::shuffleStatementCards_()
{
	std::unique_lock<std::mutex> receivedStatementCardChoicesLock(receivedStatementCardChoicesMutex);
	resetFlags = false;
	receivedStatementCardChoicesCvar.wait(receivedStatementCardChoicesLock, [this]{ return receivedStatementCardChoicesCounter == clients.size() - 1; });
	receivedStatementCardChoicesLock.unlock();

	//std::random_shuffle(playerStatementCardChoices.begin(), playerStatementCardChoices.end(), [](int i){ return rand() % i; });
	shuffledStatementCards = true;
	shuffledStatementCardsCvar.notify_all();
}

void Server::resetData_()
{
	std::unique_lock<std::mutex> sentServerConfirmationLock(sentServerConfirmationMutex);
	sentServerConfirmationCvar.wait(sentServerConfirmationLock, [this]{ return sentServerConfirmationCounter == clients.size(); });
	sentServerConfirmationLock.unlock();

	resetData();
	resetFlags = true;
	resetFlagsCvar.notify_all();
}

void Server::sendData_(int clientIndex)
{
	std::unique_lock<std::mutex> doneGeneratingLock(doneGeneratingMutex);
	doneGeneratingCvar.wait(doneGeneratingLock, [this]{ return doneGenerating; });
	doneGeneratingLock.unlock();

	int tsarIndex = game->getGameState().currentTsarIndex;
	userInterface.printMessage("Sending data to " + clients[clientIndex]->getUsername());
	sendGeneratedTsarIndexToClient(clientIndex);
	sendGeneratedPromptToClient(clientIndex);
	if (clientIndex != tsarIndex)
	{
		sendGeneratedStatementCardsToClient(clientIndex);
	}
	dataSentCounter++;
	dataSentCvar.notify_all();
}

void Server::sendStatementCardChoices_(int clientIndex)
{
	std::unique_lock<std::mutex> shuffledStatementCardsLock(shuffledStatementCardsMutex);
	shuffledStatementCardsCvar.wait(shuffledStatementCardsLock, [this]{ return shuffledStatementCards; });
	shuffledStatementCardsLock.unlock();

	sendStatementCardChoicesToClient(clientIndex);
	userInterface.printMessage("Sent player choices to " + clients[clientIndex]->getUsername());
	sentStatementCardChoicesCounter++;
	sentStatementCardChoicesCvar.notify_all();
}

void Server::sendTsarChoice_(int clientIndex)
{
	std::unique_lock<std::mutex> receivedTsarStatementCardLock(receivedTsarStatementCardMutex);
	receivedTsarStatementCardCvar.wait(receivedTsarStatementCardLock, [this]{ return receivedTsarStatementCard; });
	receivedTsarStatementCardLock.unlock();

	sendTsarStatementCardChoiceToClient(clientIndex);
	userInterface.printMessage("Sent tsar choice to " + clients[clientIndex]->getUsername());
	sentTsarStatementCardChoiceCounter++;
	sentTsarStatementCardChoiceCvar.notify_all();
}

void Server::sendConfirmation_(int clientIndex)
{
	std::unique_lock<std::mutex> receivedNextRoundConfirmationLock(receivedNextRoundConfirmationMutex);
	receivedNextRoundConfirmationCvar.wait(receivedNextRoundConfirmationLock, [this]{ return receivedNextRoundConfirmationCounter == clients.size(); });
	receivedNextRoundConfirmationLock.unlock();

	sendServerConfirmationToClient(clientIndex);
	sentServerConfirmationCounter++;
	sentServerConfirmationCvar.notify_all();

	std::unique_lock<std::mutex> resetFlagsLock(resetFlagsMutex);
	resetFlagsCvar.wait(resetFlagsLock, [this]{ return resetFlags; });
	resetFlagsLock.unlock();
}

void Server::receiveData_(int clientIndex)
{
	std::unique_lock<std::mutex> dataSentLock(dataSentMutex);
	dataSentCvar.wait(dataSentLock, [this]{ return dataSentCounter == clients.size(); });
	dataSentLock.unlock();

	int tsarIndex = game->getGameState().currentTsarIndex;

	if (clientIndex != tsarIndex)
	{
		receiveStatementCardChoiceFromClient(clientIndex);
		userInterface.printMessage("Received answer from " + clients[clientIndex]->getUsername());
		receivedStatementCardChoicesCounter++;
		receivedStatementCardChoicesCvar.notify_all();
	}
	else
	{
		std::unique_lock<std::mutex> sentStatementCardChoicesLock(sentStatementCardChoicesMutex);
		sentStatementCardChoicesCvar.wait(sentStatementCardChoicesLock, [this]{ return sentStatementCardChoicesCounter == clients.size(); });
		sentStatementCardChoicesLock.unlock();

		receiveStatementCardChoiceFromTsar(clientIndex);
		userInterface.printMessage("Received answer from tsar. - " + clients[clientIndex]->getUsername());
		receivedTsarStatementCard = true;
		receivedTsarStatementCardCvar.notify_all();
	}
}

void Server::receiveConfirmation_(int clientIndex)
{
	std::unique_lock<std::mutex> sentTsarStatementCardChoiceLock(sentTsarStatementCardChoiceMutex);
	sentTsarStatementCardChoiceCvar.wait(sentTsarStatementCardChoiceLock, [this]{ return sentTsarStatementCardChoiceCounter == clients.size(); });
	sentTsarStatementCardChoiceLock.unlock();

	receiveNextRoundConfirmationFromClient(clientIndex);
	userInterface.printMessage("Received next round confirmation from " + clients[clientIndex]->getUsername());
	receivedNextRoundConfirmationCounter++;
	receivedNextRoundConfirmationCvar.notify_all();

	std::unique_lock<std::mutex> resetFlagsLock(resetFlagsMutex);
	resetFlagsCvar.wait(resetFlagsLock, [this]{ return resetFlags; });
	resetFlagsLock.unlock();
}
