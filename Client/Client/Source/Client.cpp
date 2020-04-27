#include "Client.h"
#include "Interface.h"

#include <fstream>
#include <iostream>

Client::Client(Interface& userInterface):
	wsaManager{ WSAManager::GetInstance() },
	socket{ Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) },
	userInterface{ userInterface },
	settingsFilepath{ "settings.cfg" }
{
	loadSettings();
}

Client::~Client()
{

}

void Client::loadSettings()
{
	std::ifstream settingsFile(settingsFilepath);
	std::string settingType;

	settingsFile >> settingType >> serverIP;
	settingsFile >> settingType >> serverPort;

	settingsFile.close();
}

void Client::connectToServer()
{
	socket.Connect(IPv4Address(serverIP, serverPort));
	userInterface.printMessage("Connected to server! Waiting for other players to log in.");
}

void Client::sendUsername()
{
	int usernameLength = username.length();
	socket.Send(&usernameLength, sizeof(int));
	socket.Send(&username[0], usernameLength);
	bool validUsername = false;
	socket.Receive(&validUsername, sizeof(bool));
	if (!validUsername)
	{
		throw std::exception("Invalid username!");
	}
}

void Client::receivePlayerList()
{
	int numOfPlayers;
	socket.Receive(&numOfPlayers, sizeof(int));
	playerList.resize(numOfPlayers);
	for (auto& pair : playerList)
	{
		pair.first = 0;
	}
	for (int i = 0; i < numOfPlayers; i++)
	{
		int usernameLength;
		socket.Receive(&usernameLength, sizeof(int));
		playerList[i].second.resize(usernameLength);
		socket.Receive(&playerList[i].second[0], usernameLength);
	}
}

void Client::receiveNumOfRounds()
{
	socket.Receive(&numOfRounds, sizeof(int));
}

void Client::receiveNumOfAnswers()
{
	int numOfAnswers;
	socket.Receive(&numOfAnswers, sizeof(int));
	statementCards.resize(numOfAnswers);
}

void Client::start()
{
	sendUsername();
	receivePlayerID();
	receivePlayerList();
	receiveNumOfRounds();
	receiveNumOfAnswers();
	sendThread = std::thread(&Client::send, this);
	receiveThread = std::thread(&Client::receive, this);
	receiveThread.join();
	sendThread.join();
}

void Client::send()
{
	for (int i = 0; i < numOfRounds; i++)
	{
		sendChoice_();
		sendTsarChoice_();
		sendConfirmation_();
		resetData_();
	}
}

void Client::sendChoice_()
{
	std::unique_lock<std::mutex> dataReceivedLock(dataReceivedMutex);
	dataReceived.wait(dataReceivedLock, [this]{ return doneReceivingData; });
	if (playerID != tsarIndex)
	{
		std::string finalChoice; //TODO: move this to a separate function
		for (int j = 0; j < promptNumOfBlanks; j++)
		{
			bool validInput = false;
			std::vector<std::string> selectedStatementCards;
			while (validInput == false)
			{
				std::string choice = userInterface.requestInput();
				if (choice.length() != 1 && !isdigit(choice[0]))
				{
					userInterface.printMessage("Enter a digit corresponding to the statement card you wish to select.");
				}
				else if (choice[0] - '0' < 1 || choice[0] - '0' > statementCards.size())
				{
					userInterface.printMessage("Digit must be between 1-" + std::to_string(statementCards.size()));
				}
				else if (std::find(selectedStatementCards.begin(), selectedStatementCards.end(), choice) != selectedStatementCards.end())
				{
					userInterface.printMessage("Statement card already chosen.");
				}
				else
				{
					selectedStatementCards.emplace_back(choice);
					finalChoice += choice;
					validInput = true;
				}
			}
		}
		sendChoice(finalChoice);
		sentChoice = true;
	}
	choiceSent.notify_all();
}

void Client::sendTsarChoice_()
{
	if (playerID == tsarIndex)
	{
		std::unique_lock<std::mutex> choicesReceivedLock(choicesReceivedMutex);
		choicesReceived.wait(choicesReceivedLock, [this]{ return receivedChoices; });
		choicesReceivedLock.unlock();
		bool validInput = false;
		std::string	choice;
		while (validInput == false)
		{
			choice = userInterface.requestInput();
			if (choice.length() != 1 && !isdigit(choice[0]))
			{
				userInterface.printMessage("Enter a digit corresponding to the answer you wish to select.");
			}
			else if (choice[0] - '0' < 1 || choice[0] - '0' > playerList.size() - 1)
			{
				userInterface.printMessage("Digit must be between 1-" + std::to_string(statementCards.size()));
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
}

void Client::sendConfirmation_()
{
	std::unique_lock<std::mutex> tsarChoiceReceivedLock(tsarChoiceReceivedMutex);
	tsarChoiceReceived.wait(tsarChoiceReceivedLock, [this]{ return receivedTsarChoice; });
	tsarChoiceReceivedLock.unlock();
	confirmNextRound();
	sentConfirmation = true;
	confirmationSent.notify_all();
}

void Client::resetData_()
{
	std::unique_lock<std::mutex> receivedConfirmationLock(confirmationReceivedMutex);
	confirmationReceived.wait(receivedConfirmationLock, [this]{return receivedConfirmation; });
	receivedConfirmationLock.unlock();
	resetData();
	flagsReset = true;
	resetFlags.notify_all();
}

void Client::receive()
{
	for (int i = 0; i < numOfRounds; i++)
	{
		receiveData_(i);
		receiveStatementCardChoices_();
		receiveTsarChoice_();
		receiveConfirmation_();
	}
}

void Client::receiveData_(int i)
{
	receiveTsarIndex();
	receivePrompt();
	if (playerID != tsarIndex)
	{
		receiveStatementCards();
	}
	doneReceivingData = true;
	displayInformation(i);
	dataReceived.notify_all();
}

void Client::receiveStatementCardChoices_()
{
	std::unique_lock<std::mutex> choiceSentLock(choiceSentMutex);
	choiceSent.wait(choiceSentLock, [this]{ return sentChoice || playerID == tsarIndex; });
	choiceSentLock.unlock();
	receiveStatementCardChoices();
	displayChoices();
	receivedChoices = true;
	choicesReceived.notify_all();
}

void Client::receiveTsarChoice_()
{
	std::unique_lock<std::mutex> tsarChoiceSentLock(m_tsarChoiceSentMutex);
	m_tsarChoiceSent.wait(tsarChoiceSentLock, [this]{ return m_sentTsarChoice || playerID != tsarIndex; });
	tsarChoiceSentLock.unlock();
	receiveTsarChoice();
	displayTsarChoice();
	receivedTsarChoice = true;
	tsarChoiceReceived.notify_all();
}

void Client::receiveConfirmation_()
{
	std::unique_lock<std::mutex> confirmationSentLock(confirmationSentMutex);
	confirmationSent.wait(confirmationSentLock, [this]{ return sentConfirmation; });
	confirmationSentLock.unlock();
	receiveServerConfirmation();
	receivedConfirmation = true;
	confirmationReceived.notify_all();
	std::unique_lock<std::mutex> resetFlagsLock(resetFlagsMutex);
	resetFlags.wait(resetFlagsLock, [this]{ return flagsReset; });
	resetFlagsLock.unlock();
}


void Client::sendChoice(const std::string& choice)
{
	for (int i = 0; i < promptNumOfBlanks; i++)
	{
		int choiceInt = choice[i] - '0' - 1;
		socket.Send(&choiceInt, sizeof(int));
		int answerLength = statementCards[choiceInt].length();
		socket.Send(&answerLength, sizeof(int));
		socket.Send(&statementCards[choiceInt][0], answerLength);
	}
}

void Client::sendTsarChoice(const std::string& choice)
{
	int choiceInt = atoi(choice.c_str()) - 1;
	socket.Send(&choiceInt, sizeof(int));
}

void Client::receivePlayerID()
{
	socket.Receive(&playerID, sizeof(int));
}

void Client::receiveTsarIndex()
{
	socket.Receive(&tsarIndex, sizeof(int));
}

void Client::receivePrompt()
{
	int questionLength;
	socket.Receive(&questionLength, sizeof(int));
	prompt.resize(questionLength);
	socket.Receive(&prompt[0], questionLength);
	socket.Receive(&promptNumOfBlanks, sizeof(int));
}

void Client::receiveStatementCards()
{
	int numOfAnswers;
	socket.Receive(&numOfAnswers, sizeof(int));
	for (int i = 0; i < numOfAnswers; i++)
	{
		int questionLength;
		socket.Receive(&questionLength, sizeof(int));
		std::string question;
		question.resize(questionLength);
		socket.Receive(&question[0], questionLength);
		statementCards[i] = question;
	}
}

void Client::receiveStatementCardChoices()
{
	int numOfAnswers;
	socket.Receive(&numOfAnswers, sizeof(int));
	for (int i = 0; i < numOfAnswers; i++)
	{
		std::vector<std::string> answers;
		for (int j = 0; j < promptNumOfBlanks; j++)
		{
			int answerLength;
			std::string answer;
			socket.Receive(&answerLength, sizeof(int));
			answer.resize(answerLength);
			socket.Receive(&answer[0], answerLength);
			answers.emplace_back(answer);
		}
		statementCardChoices.emplace_back(answers);
	}
}

void Client::receiveTsarChoice()
{
	socket.Receive(&winnerIndex, sizeof(int));
	playerList[winnerIndex].first++;
	socket.Receive(&tsarChoiceIndex, sizeof(int));
}

void Client::receiveServerConfirmation()
{
	bool confirmation;
	socket.Receive(&confirmation, sizeof(bool));
}

void Client::displayInformation(int round)
{
	system("cls");
	userInterface.printMessage("Round #" + std::to_string(round + 1));
	userInterface.printMessage("Player: " + playerList[playerID].second);
	userInterface.printMessage("Player list:");
	for (int i = 0; i < playerList.size(); i++)
	{
		if (i == tsarIndex)
		{
			userInterface.printMessage(playerList[i].second + " - " + std::to_string(playerList[i].first) + " - Tsar");
		}
		else
		{
			userInterface.printMessage(playerList[i].second + " - " + std::to_string(playerList[i].first));
		}
	}
	userInterface.printMessage(prompt + "\n");
	if (playerID != tsarIndex)
	{
		for (int i = 0; i < statementCards.size(); i++)
		{
			userInterface.printMessage(std::to_string(i + 1) + ". " + statementCards[i]);
		}
	}
}

void Client::displayChoices()
{
	userInterface.printMessage("Player choices:\n");
	for (auto& choice : statementCardChoices)
	{
		std::string completeChoice;
		for (int i = 0; i < choice.size() - 1; i++)
		{
			completeChoice += choice[i] + " / ";
		}
		completeChoice += choice[choice.size() - 1];
		userInterface.printMessage(completeChoice);
	}
}

void Client::displayTsarChoice()
{
	userInterface.printMessage("Tsar choice:\n");
	std::string completeChoice;
	for (int i = 0; i < statementCardChoices[tsarChoiceIndex].size() - 1; i++)
	{
		completeChoice += statementCardChoices[tsarChoiceIndex][i] + " / ";
	}
	completeChoice += statementCardChoices[tsarChoiceIndex][statementCardChoices[tsarChoiceIndex].size() - 1];
	userInterface.printMessage(completeChoice);
}

void Client::confirmNextRound()
{
	userInterface.printMessage("Press any key to continue:");
	while (userInterface.requestInput().length() == 0);
	bool confirmed;
	socket.Send(&confirmed, sizeof(bool));
}

void Client::resetData()
{
	doneReceivingData = false;
	sentChoice = false;
	receivedChoices = false;
	m_sentTsarChoice = false;
	receivedTsarChoice = false;
	flagsReset = false;
	sentConfirmation = false;
	receivedConfirmation = false;
	statementCardChoices.clear();
}