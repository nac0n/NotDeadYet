/*
	Made by Jensen Christopher Datum(21/11/12 created) 
	for project Not Dead Yet at Blekinge tekniska h�gskola
*/
#pragma once
#include "Process.h"
#include "GraphicsEngine.h"
#include "GameFiles/ClientSide/Player.h"
#include "Network/ServerChannel.h"
#include "GameFiles/KeyUtil/KeyHandler.h"
#include "Network/NetworkMessageConverter.h"

using namespace MaloW;

class Client : public MaloW::Process
{
private:
	int zID;

	int	zPort;
	std::string zIP;

	INT64 zStartime;
	float zDeltaTime;
	float zSecsPerCnt;

	KeyHandler zKeyInfo;
	std::string zMeshID;
	GraphicsEngine* zEng;

	float zTimeSinceLastPing;

	std::vector<Player*> zPlayers;
	NetworkMessageConverter zMsgHandler;
	ServerChannel* zServerChannel;
private:
	/*! Handle Keyboard Input */
	void HandleKeyboardInput();
	/*! Splits Network Message and detects what type was sent*/
	void HandleNetworkMessage(std::string msg);
	/*! Handles Message types*/
	void HandleNewPlayer(const std::vector<std::string>& msgArray);
	void HandlePlayerUpdate(const std::vector<std::string>& msgArray);
	void HandleRemovePlayer(const std::vector<std::string>& msgArray);
	/*! Pings client to check if server is still running*/
	void Ping();
	/*! Close the connection and print the reason to the client*/
	void CloseConnection(const std::string reason);
	/*! Initializes the graphic stuff*/
	void initClient();
	float Update();
	/*! Check if the key is pressed and potentially sends a message to the server*/
	bool CheckKey(const unsigned int ID);
public:
	Client();
	/*! Connects to a Host with the specified parameters*/
	int Connect(const std::string& ip, const int port);
	virtual ~Client();
	void Life();
	/*! Checks if Thread is alive*/
	bool IsAlive();
};