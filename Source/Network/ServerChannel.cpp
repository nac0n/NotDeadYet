#include "ServerChannel.h"
#include "NetworkPacket.h"
#include "Winsock.h"
#include "NetworkMessageConverter.h"
#include "ClientServerMessages.h"

using namespace MaloW;

ServerChannel::ServerChannel()
{
	this->stayAlive = false;
	this->notifier = NULL;
}

int ServerChannel::InitConnection(std::string IP, int port)
{
	int returnCode = 0;
	this->stayAlive = true;
	this->notifier = NULL;

	WSADATA wsaData;
	int retCode = WSAStartup(MAKEWORD(2,2), &wsaData);
	if(retCode != 0) 
	{
		returnCode = 2;
		this->stayAlive = false;
		MaloW::Debug("SC: Failed to init Winsock library. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
		WSACleanup();
	}

	// open a socket
	this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if(this->sock == INVALID_SOCKET) 
	{
		returnCode = 3;
		this->stayAlive = false;
		MaloW::Debug("SC: Invalid socket, failed to create socket. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
		WSACleanup();
	}

	// connect
	sockaddr_in saServer;
	saServer.sin_port = htons((u_short)port);
	saServer.sin_addr.s_addr = inet_addr(IP.c_str());
	saServer.sin_family = AF_INET;
	retCode = connect(this->sock, (sockaddr*)&saServer, sizeof(saServer));
	if(retCode == SOCKET_ERROR)
	{
		returnCode = 1;
		this->stayAlive = false;
		MaloW::Debug("SC: Connection to server failed. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
		WSACleanup();
	}

	this->buffer = "";
	this->unImportantFilter = "";

	return returnCode;
}

ServerChannel::~ServerChannel()
{
	this->Close();
	this->WaitUntillDone();

	int retCode = 0;

	if(this->sock != 0)
	{
		// close server socket
		retCode = closesocket(this->sock);
		if(retCode == SOCKET_ERROR) 
			MaloW::Debug("SC: Error failed to close socket. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
		
	}
	// Release WinSock DLL
	retCode = WSACleanup();
	if(retCode == SOCKET_ERROR) 
		MaloW::Debug("SC: Error cleaning up Winsock Library. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));;

}

string ServerChannel::receiveData()
{
	std::string msg = "";
	bool getNewData = true;
	if(!this->buffer.empty())
	{
		if(int pos = this->buffer.find(10))
		{
			msg = this->buffer.substr(0, pos);
			this->buffer = this->buffer.substr(pos+1, this->buffer.length());
			getNewData = false;
		}
	}
	if(getNewData)
	{
		bool goAgain = true;
		do
		{
			char bufs[1024] = {0};
			int retCode = 0;
			retCode = recv(this->sock, bufs, sizeof(bufs), 0);
			if(retCode == SOCKET_ERROR)
			{
				this->Close();
				MaloW::Debug("SC: Error receiving data. Error: " + MaloW::convertNrToString((float)WSAGetLastError()) + ". Probably due to crash/improper disconnect");
			}
			else if(retCode == 0)
			{
				this->Close();
				MaloW::Debug("SC: Server disconnected, closing.");
			}
			else
			{
				for(int i = 0; i < 1024; i++)
				{
					if(bufs[i] == 10)
						goAgain = false;
					if(bufs[i] != 0)
						this->buffer += bufs[i];
					else
						i = 1024;
				}
				if(!goAgain)
				{
					for(int i = 0; i < 1024; i++)
					{
						if(this->buffer[i] != 10)
							msg += this->buffer[i];
						else
						{
							this->buffer = this->buffer.substr(i+1, this->buffer.length());
							i = 1024;
						}
					}
				}
			}
		}
		while(goAgain && this->stayAlive);
	}
	if (Messages::FileWrite())
		Messages::Debug("SC: Received from Server " + msg);
	return msg;
}

void ServerChannel::sendData(string msg)
{
	if (Messages::FileWrite())
		Messages::Debug("Sc: Sending to Server " + msg);

	msg += 10;
	char bufs[1024] = {0};
	for(unsigned int i = 0; i < msg.length(); i++)
	{
		bufs[i] = msg[i];
	}
	int retCode = send(this->sock, bufs, sizeof(bufs), 0);
	if(retCode == SOCKET_ERROR)
	{
		MaloW::Debug("SC: Error sending data. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
	}
}

void ServerChannel::Life()
{
	MaloW::Debug("ServerChannel Process Started");

	while(this->stayAlive)
	{
		string msg = this->receiveData();
		if(msg != "")
		{
			if(this->notifier && this->stayAlive)
			{
				NetworkPacket* np = new NetworkPacket(msg, 0);
				this->notifier->PutEvent(np);
			}
		}
	}
}

void ServerChannel::CloseSpecific()
{
	if(this->sock != 0)
	{
		int retCode = shutdown(this->sock, 2); // 2 = SD_BOTH
		if(retCode == SOCKET_ERROR) 
			MaloW::Debug("SC: Error trying to perform shutdown on socket from a ->Close() call. Error: " + MaloW::convertNrToString((float)WSAGetLastError()));
	}
	this->sock = 0;
}