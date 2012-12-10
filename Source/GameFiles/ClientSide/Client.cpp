#include "GameFiles/ClientSide/Client.h"
#include "Graphics.h"
#include "Safe.h"
#include "Network/NetworkPacket.h"

using namespace MaloW;

//Timeout_value = 10 sek
static const float TIMEOUT_VALUE = 10.0f; 
static const float UPDATE_DELAY = 0.0333f;

enum MSG_TYPE
{
	PLAYER,
	STATIC_OBJECT,
	ANIMAL
};
Client::Client()
{
	this->zID = 0;
	this->zIP = "";
	this->zPort = 0;
	this->zEng = NULL;
	this->zCreated = false;
	this->zRunning = true;
	this->zFrameTime = 0.0f;
	this->zWaitTimer = 0.0f;
	this->zKeyInfo = KeyHandler();
	this->zKeyInfo.InitKeyBinds();
	this->zServerChannel = NULL;
	this->zTimeSinceLastPing = 0.0f;
	this->zMsgHandler = NetworkMessageConverter();

	zMeshID = "Media/scale.obj";

	INT64 frequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);

	this->zSecsPerCnt = 1.0f / (float)(frequency);

	QueryPerformanceCounter((LARGE_INTEGER*)&this->zStartime);
}
int Client::Connect(const std::string& ip, const int port)
{
	int code = 0;

	this->zIP = ip;
	this->zPort = port;
	this->zEng = GetGraphics();
	this->zServerChannel = new ServerChannel();
	//Tries to Connect to a server with the specified Ip and Port
	code = this->zServerChannel->InitConnection(ip, port);
	
	return code;
}
Client::~Client()
{
	this->Close();
	this->WaitUntillDone();
	for( auto x = zPlayers.begin(); x < zPlayers.end(); ++x )
	{
		SAFE_DELETE(*x);
	}

	SAFE_DELETE(this->zServerChannel);
}
float Client::Update()
{
	INT64 currentTime;
	float timeDifference;

	QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

	timeDifference = (float)(currentTime - this->zStartime);

	this->zDeltaTime = timeDifference * this->zSecsPerCnt;

	this->zStartime = currentTime;

	this->zFrameTime += this->zDeltaTime;

	return this->zDeltaTime;
}
void Client::initClient()
{
	this->zEng->CreateSkyBox("Media/skymap.dds");
	this->zEng->GetCamera()->SetPosition( Vector3(1, 4, -1) );
	this->zEng->GetCamera()->LookAt( Vector3(10, 10, 10) );

	this->zEng->StartRendering();

	//this->zEng->CreateTerrain(Vector3(0, 0, 0), Vector3(100, 1, 100), "Media/TerrainTexture.png", "Media/TerrainHeightmap.raw");

	//StaticMesh* scaleHuman = this->zEng->CreateStaticMesh("Media/scale.obj", D3DXVECTOR3(5, -6, 15));
	//scaleHuman->Scale(0.1f);
	//this->zEng->GetCamera()->FollowMesh(scaleHuman);

	//iLight* testLight = this->zEng->CreateLight(Vector3(15, 30, 15));
	//testLight->SetIntensity(50);
	//SoundEngine* seng = eng->GetSoundEngine();
	//seng->SetMasterVolume(0.5f);

	//this->zEng->LoadingScreen("Media/LoadingScreen/StartScreen.png", "", 0.0f, 1.0f, 1.0f, 1.0f);
}
void Client::Life()
{
	this->Update();
	this->zServerChannel->setNotifier(this);

	this->zServerChannel->Start();

	this->initClient();
	Sleep(1000);
	while(this->zEng->IsRunning() && this->stayAlive)
	{
		this->Update();

		this->zWaitTimer += this->zDeltaTime;
		this->zTimeSinceLastPing += this->zDeltaTime;

		this->HandleKeyboardInput();
		if (this->stayAlive)
		{
			this->UpdateCameraPos();

			this->UpdateWorldObjects();

			if (MaloW::ProcessEvent* ev = this->PeekEvent())
			{
				//Check if Client has received a Message
				NetworkPacket* np = dynamic_cast<NetworkPacket*>(ev);
				if (np != NULL)
				{
					this->HandleNetworkMessage(np->getMessage());
				}
				SAFE_DELETE(ev);
			}
			if (this->zTimeSinceLastPing > TIMEOUT_VALUE * 3.0f)
			{
				this->CloseConnection("Timeout");
			}
			else if (this->zTimeSinceLastPing > TIMEOUT_VALUE)
			{
				//Print a Timeout Message to Client
			}
			if(this->zCreated)
			{
				if(this->zWaitTimer >= UPDATE_DELAY)
				{
					this->zWaitTimer = 0.0f;
					this->SendClientUpdate();
				}
			}
			Sleep(5);
		}
	}
	this->zRunning = false;
}
void Client::SendClientUpdate()
{
	std::string msg;
	Vector3 dir = this->zEng->GetCamera()->GetForward();
	Vector3 up = this->zEng->GetCamera()->GetUpVector();

	msg = this->zMsgHandler.Convert(MESSAGE_TYPE_CLIENT_DATA);
	msg += this->zMsgHandler.Convert(MESSAGE_TYPE_FRAME_TIME, this->zFrameTime);
	msg += this->zMsgHandler.Convert(MESSAGE_TYPE_DIRECTION, dir.x, dir.y, dir.z);
	msg += this->zMsgHandler.Convert(MESSAGE_TYPE_UP, up.x, up.y, up.z);
	msg += this->zMsgHandler.Convert(MESSAGE_TYPE_ROTATION, 0, 0, 0, 0);

	this->zServerChannel->sendData(msg);
}
void Client::UpdateCameraPos()
{
	int pos = this->SearchForPlayer(this->zID);
	if (pos != -1)
	{
		Vector3 position = this->zPlayers[pos]->GetObjectPosition();
		position.y += 2.5f;
		this->zEng->GetCamera()->SetPosition(position);
	}
}
void Client::UpdateWorldObjects()
{
	std::vector<Player*>::iterator itp;
	for (itp = this->zPlayers.begin(); itp < this->zPlayers.end(); itp++)
	{
		(*itp)->Update(this->zDeltaTime);
	}

	std::vector<Animal*>::iterator ita;
	for (ita = this->zAnimals.begin(); ita < this->zAnimals.end(); ita++)
	{
		(*ita)->Update(this->zDeltaTime);
	}
}
bool Client::IsAlive()
{
	return this->zRunning;
}
bool Client::CheckKey(const unsigned int ID)
{
	bool result = false;
	char key = this->zKeyInfo.GetKey(ID);
	//Check if key is pressed
	if (this->zEng->GetKeyListener()->IsPressed(key))
	{
		//Check if the Key was pressed last frame
		if (!this->zKeyInfo.GetKeyState(ID))
		{
			std::string msg = "";
			msg = this->zMsgHandler.Convert(MESSAGE_TYPE_KEY_DOWN, ID);

			this->zServerChannel->sendData(msg);
		}
		this->zKeyInfo.SetKeyState(ID, true);
		result = true;
	}
	else 
	{
		//Check if the Key was pressed last frame
		if (this->zKeyInfo.GetKeyState(ID))
		{
			this->zServerChannel->sendData(this->zMsgHandler.Convert(MESSAGE_TYPE_KEY_UP, ID));
		}
		this->zKeyInfo.SetKeyState(ID, false);
		result = false;
	}
	return result;
}
void Client::HandleKeyboardInput()
{
	float mSpeed = V_WALK_SPEED;
	bool pressed = false;
	pressed = this->CheckKey(KEY_FORWARD);

	int pos = this->SearchForPlayer(this->zID);
	if (pos != -1)
	{
		Player* player = this->zPlayers[pos];
		switch (player->GetPlayerState())
		{
		case STATE_WALKING:
			mSpeed = V_WALK_SPEED;
			break;
		case STATE_RUNNING:
			mSpeed = V_RUN_SPEED;
			break;
		case STATE_CROUCHING:
			mSpeed = V_CROUCH_SPEED;
			break;
		default:
			mSpeed = V_WALK_SPEED;
			break;
		}
		if (pressed)
		{
			Vector3 camForward = this->zEng->GetCamera()->GetForward();

			Vector3 position = this->zPlayers.at(pos)->GetObjectPosition();
			Vector3 newPos = position + (camForward * this->zDeltaTime * mSpeed);
			//player->SetNextPosition(newPos);
		}
		else
		{
			pressed = this->CheckKey(KEY_BACKWARD);
			if (pressed)
			{
				Vector3 camBackwards = this->zEng->GetCamera()->GetForward() * -1;

				Vector3 position = player->GetObjectPosition();
				Vector3 newPos = position + (camBackwards * this->zDeltaTime * mSpeed);

				//player->SetNextPosition(newPos);
			}
		}

		pressed = this->CheckKey(KEY_LEFT);
		if (pressed)
		{
			Vector3 camForward = this->zEng->GetCamera()->GetForward();
			Vector3 camUp = this->zEng->GetCamera()->GetUpVector();
			Vector3 camRight = camForward.GetCrossProduct(camUp);

			Vector3 position = this->zPlayers.at(pos)->GetObjectPosition();
			Vector3 newPos = position + (camRight * this->zDeltaTime * mSpeed);

			//player->SetNextPosition(newPos);
		}
		else
		{
			pressed = this->CheckKey(KEY_RIGHT);
			if (pressed)
			{
				Vector3 camForward = this->zEng->GetCamera()->GetForward();
				Vector3 camUp = this->zEng->GetCamera()->GetUpVector();
				Vector3 camRight = camForward.GetCrossProduct(camUp) * -1;

				Vector3 position = this->zPlayers.at(pos)->GetObjectPosition();
				Vector3 newPos = position + (camRight * this->zDeltaTime * mSpeed);
				//player->SetNextPosition(newPos);
			}
		}

		pressed = this->CheckKey(KEY_SPRINT);
		if (pressed)
		{
			if (!this->zKeyInfo.GetKeyState(KEY_SPRINT))
			{
				if (this->zPlayers[pos]->GetPlayerState() != STATE_RUNNING)
					this->zPlayers[pos]->SetPlayerState(STATE_RUNNING);
				else
					this->zPlayers[pos]->SetPlayerState(STATE_WALKING);
			}
		}

		pressed = this->CheckKey(KEY_DUCK);
		if (pressed)
		{
			if (!this->zKeyInfo.GetKeyState(KEY_DUCK))
			{
				if (this->zPlayers[pos]->GetPlayerState() != STATE_CROUCHING)
					this->zPlayers[pos]->SetPlayerState(STATE_CROUCHING);
				else
					this->zPlayers[pos]->SetPlayerState(STATE_WALKING);
			}
		}
	}

	if (this->zEng->GetKeyListener()->IsPressed(this->zKeyInfo.GetKey(KEY_MENU)))
	{
		this->zServerChannel->sendData(this->zMsgHandler.Convert(MESSAGE_TYPE_CONNECTION_CLOSED, this->zID));
		this->CloseConnection("Escape was pressed");
	}
}
void Client::Ping()
{
	this->zTimeSinceLastPing = 0.0f;
	this->zServerChannel->sendData(this->zMsgHandler.Convert(MESSAGE_TYPE_PING));
}
void Client::HandleNetworkMessage(const std::string& msg)
{
	std::vector<std::string> msgArray;
	msgArray = this->zMsgHandler.SplitMessage(msg);
	char key[1024];
	if(msgArray.size() > 0)
	{
		sscanf(msgArray[0].c_str(), "%s ", key);

		//Checks what type of message was sent
		if(strcmp(key, PING.c_str()) == 0)
		{
			this->Ping();
		}
		//Player
		else if(strcmp(key, UPDATE_PLAYER.c_str()) == 0)
		{
			this->HandleUpdateObject(msgArray, PLAYER);
		}
		else if(strcmp(key, NEW_PLAYER.c_str()) == 0)
		{
			this->HandleNewObject(msgArray, PLAYER);
		}
		else if(strcmp(key, REMOVE_PLAYER.c_str()) == 0)
		{
			this->HandleRemoveObject(msgArray, PLAYER);
		}
		//Static Object
		else if(strcmp(key, UPDATE_OBJECT.c_str()) == 0)
		{
			this->HandleUpdateObject(msgArray, STATIC_OBJECT);
		}
		else if(strcmp(key, NEW_OBJECT.c_str()) == 0)
		{
			this->HandleNewObject(msgArray, STATIC_OBJECT);
		}
		else if(strcmp(key, REMOVE_OBJECT.c_str()) == 0)
		{
			this->HandleRemoveObject(msgArray, STATIC_OBJECT);
		}
		//Animal
		else if(strcmp(key, UPDATE_ANIMAL.c_str()) == 0)
		{
			this->HandleUpdateObject(msgArray, ANIMAL);
		}
		else if(strcmp(key, NEW_ANIMAL.c_str()) == 0)
		{
			this->HandleNewObject(msgArray, ANIMAL);
		}
		else if(strcmp(key, REMOVE_ANIMAL.c_str()) == 0)
		{
			this->HandleRemoveObject(msgArray, ANIMAL);
		}
		else if(strcmp(key, SELF_ID.c_str()) == 0)
		{
			this->zID = this->zMsgHandler.ConvertStringToInt(SELF_ID, msgArray[0]);
			
			Vector3 camDir = this->zEng->GetCamera()->GetForward();
			Vector3 camUp = this->zEng->GetCamera()->GetUpVector();

			std::string serverMessage = "";
			serverMessage = this->zMsgHandler.Convert(MESSAGE_TYPE_USER_DATA);
			serverMessage += this->zMsgHandler.Convert(MESSAGE_TYPE_MESH_MODEL, this->zMeshID);
			serverMessage += this->zMsgHandler.Convert(MESSAGE_TYPE_DIRECTION, camDir.x, camDir.y, camDir.z);
			serverMessage += this->zMsgHandler.Convert(MESSAGE_TYPE_UP, camUp.x, camUp.y, camUp.z);

			this->zServerChannel->sendData(serverMessage);
		}
		else if(strcmp(key, SERVER_FULL.c_str()) == 0)
		{
			this->CloseConnection("Server is full");
		}
		else if(strcmp(key, KICKED.c_str()) == 0)
		{
			this->CloseConnection("You got kicked");
		}
		else if(strcmp(key, SERVER_SHUTDOWN.c_str()) == 0)
		{
			this->CloseConnection("Server Shutdown");
		}
		else
		{
			MaloW::Debug("C: Unknown Message Was sent from server " + msgArray[0] + " in HandleNetworkMessage");
		}
	}
}
void Client::CloseConnection(const std::string& reason)
{
	MaloW::Debug("Client Shutdown:" + reason);
	//Todo Skriv ut vilket reason som gavs
	this->zServerChannel->Close();
	this->Close();
}
int Client::SearchForPlayer(const int id)
{
	int position = -1;
	bool found = false;

	for (unsigned int i = 0; i < this->zPlayers.size() && !found; i++)
	{
		if (this->zPlayers[i]->GetID() == id)
		{
			position = i;
			found = true;
		}
	}

	return position;
}
int Client::SearchForObject(const int id)
{
	int position = -1;
	bool found = false;

	for (unsigned int i = 0; i < this->zStaticObjects.size() && !found; i++)
	{
		if (this->zStaticObjects[i]->GetID() == id)
		{
			position = i;
			found = true;
		}
	}

	return position;
}
int Client::SearchForAnimal(const int id)
{
	int position = -1;
	bool found = false;
	
	for (unsigned int i = 0; i < this->zAnimals.size() && !found; i++)
	{
		if (this->zAnimals[i]->GetID() == id)
		{
			position = i;
			found = true;
		}
	}

	return position;
}
int Client::FindObject(const int id, const unsigned int objectType)
{
	int position = -1;
	switch (objectType)
	{
	case PLAYER:
		position = SearchForPlayer(id);
		break;
	case ANIMAL:
		position = SearchForAnimal(id);
		break;
	case STATIC_OBJECT:
		position = SearchForObject(id);
		break;
	default:
		position = -1;
		break;
	}
	return position;
}
void Client::HandleNewObject(const std::vector<std::string>& msgArray, const unsigned int objectType )
{
	Vector3 position = Vector3(0, 0, 0);
	Vector3 scale = Vector3(0.05f, 0.05f, 0.05f);
	Vector4 rotation = Vector4(0, 0, 0, 0);
	Vector3 direction = Vector3(1, 0, 0);
	std::string filename = "";
	int clientID = -1;
	int state = 0;

	char key[512];
	for(unsigned int i = 0; i < msgArray.size(); i++)
	{
		sscanf(msgArray[i].c_str(), "%s ", key);

		if(strcmp(key, NEW_PLAYER.c_str()) == 0)
		{
			switch (objectType)
			{
			case PLAYER:
				clientID = this->zMsgHandler.ConvertStringToInt(NEW_PLAYER, msgArray[i]);
				break;
			case ANIMAL:
				clientID = this->zMsgHandler.ConvertStringToInt(NEW_ANIMAL, msgArray[i]);
				break;
			case STATIC_OBJECT:
				clientID = this->zMsgHandler.ConvertStringToInt(NEW_OBJECT, msgArray[i]);
				break;
			default:
				break;
			}
		}
		else if(strcmp(key, POSITION.c_str()) == 0)
		{
			position = this->zMsgHandler.ConvertStringToVector(POSITION, msgArray[i]);
		}
		else if(strcmp(key, ROTATION.c_str()) == 0)
		{
			rotation = this->zMsgHandler.ConvertStringToQuaternion(ROTATION, msgArray[i]);
		}
		else if(strcmp(key, STATE.c_str()) == 0)
		{
			state = this->zMsgHandler.ConvertStringToInt(STATE, msgArray[i]);
		}
		else if(strcmp(key, SCALE.c_str()) == 0)
		{
			scale = this->zMsgHandler.ConvertStringToVector(SCALE, msgArray[i]);
		}
		else if(strcmp(key, MESH_MODEL.c_str()) == 0)
		{
			filename = this->zMsgHandler.ConvertStringToSubstring(MESH_MODEL, msgArray[i]);
		}
		else if (strcmp(key, DIRECTION.c_str()) == 0)
		{
			direction = this->zMsgHandler.ConvertStringToVector(DIRECTION, msgArray[i]);
		}
		else
		{
			MaloW::Debug("C: Unknown Message Was sent from server " + msgArray[i] + " in HandleNewObject");
		}
	}
	if (!this->zCreated)
	{
		if (clientID == this->zID)
		{
			this->zCreated = true;
		}
	}
	if (clientID != -1)
	{
		int pos;
		switch (objectType)
		{
		case PLAYER:
			pos = this->SearchForPlayer(clientID);
			break;
		case ANIMAL:
			pos = this->SearchForAnimal(clientID);
			break;
		case STATIC_OBJECT:
			pos = this->SearchForObject(clientID);
			break;
		default:
			pos = -1;
			break;
		}
		if (pos == -1)
		{
			WorldObject* newWorldObject = NULL;
			switch (objectType)
			{
			case PLAYER:
				newWorldObject = new Player();
				break;
			case ANIMAL:
				newWorldObject = new Animal();
				break;
			case STATIC_OBJECT:
				newWorldObject = new StaticObject();
				break;
			default:
				break;
			}
			//Creates a StaticMesh from the given Filename
			iMesh* mesh = this->zEng->CreateStaticMesh(filename.c_str(), position);
			mesh->SetQuaternion(rotation);
			mesh->Scale(scale);
		
			//Create player data
			newWorldObject->AddStaticMesh(mesh);
			newWorldObject->SetID(clientID);
			switch (objectType)
			{
			case PLAYER:
				dynamic_cast<Player*>(newWorldObject)->SetNextPosition(position);
				dynamic_cast<Player*>(newWorldObject)->SetPlayerState(state);
				this->zPlayers.push_back(dynamic_cast<Player*>(newWorldObject));
				break;
			case STATIC_OBJECT:
				this->zStaticObjects.push_back(dynamic_cast<StaticObject*>(newWorldObject));
				break;
			case ANIMAL:
				dynamic_cast<Animal*>(newWorldObject)->SetNextPosition(position);
				dynamic_cast<Animal*>(newWorldObject)->SetAnimalState(state);
				this->zAnimals.push_back(dynamic_cast<Animal*>(newWorldObject));
				break;
			default:
				break;
			}
		}
		else
		{
			MaloW::Debug("Cant create a new Object. It already exists");
		}
	}
}
void Client::HandleRemoveObject(const std::vector<std::string>& msgArray, const unsigned int objectType )
{
	int ID = -1;
	int pos = -1;
	switch(objectType)
	{
	case PLAYER:
		ID = this->zMsgHandler.ConvertStringToInt(REMOVE_PLAYER, msgArray[0]);
		pos = SearchForPlayer(ID);
		break;
	case ANIMAL:
		ID = this->zMsgHandler.ConvertStringToInt(REMOVE_ANIMAL, msgArray[0]);
		pos = SearchForAnimal(ID);
		break;
	case STATIC_OBJECT:
		ID = this->zMsgHandler.ConvertStringToInt(REMOVE_OBJECT, msgArray[0]);
		pos = SearchForObject(ID);
		break;
	default:
		break;
	}
	//Check if client was found in the array
	if(pos != -1)
	{
		if(ID == this->zID)
		{
			this->CloseConnection("Unknown reason possible Kicked");
		}
		switch(objectType)
		{
		case PLAYER:
			this->zEng->DeleteMesh(this->zPlayers[pos]->GetObjectMesh());
			this->zPlayers.erase(zPlayers.begin() + pos);
			break;
		case ANIMAL:
			this->zEng->DeleteMesh(this->zAnimals[pos]->GetObjectMesh());
			this->zAnimals.erase(zAnimals.begin() + pos);
			break;
		case STATIC_OBJECT:
			this->zEng->DeleteMesh(this->zStaticObjects[pos]->GetObjectMesh());
			this->zStaticObjects.erase(zStaticObjects.begin() + pos);
			break;
		default:
			break;
		}
	}
}
void Client::HandleUpdateObject(const std::vector<std::string>& msgArray, const unsigned int objectType)
{
	int ID = -1;
	int pos = -1;
	//Get ID and Position Depending on type
	switch (objectType)
	{
	case PLAYER:
		ID = this->zMsgHandler.ConvertStringToInt(UPDATE_PLAYER, msgArray[0]);
		pos = SearchForPlayer(ID);
		break;
	case ANIMAL:
		ID = this->zMsgHandler.ConvertStringToInt(UPDATE_ANIMAL, msgArray[0]);
		pos = SearchForAnimal(ID);
		break;
	case STATIC_OBJECT:
		ID = this->zMsgHandler.ConvertStringToInt(UPDATE_OBJECT, msgArray[0]);
		pos = SearchForObject(ID);
		break;
	default:
		pos = -1;
		MaloW::Debug("Wrong ObjectType in Client::HandleUpdate()");
		break;
	}
	//Check if object was found
	if(pos != -1)
	{
		WorldObject* worldObjectPointer = NULL;
		switch (objectType)
		{
		case PLAYER:
			worldObjectPointer = this->zPlayers[pos];
			break;
		case ANIMAL:
			worldObjectPointer = this->zAnimals[pos];
			break;
		case STATIC_OBJECT:
			worldObjectPointer = this->zStaticObjects[pos];
			break;
		default:
			break;
		}
		Vector3 position;
		Vector4 rotation;
		int state;
		std::string filename;
		float serverTime = 0.0f;

		bool bState = false;
		bool bPos = false;
		bool bRot = false;
		bool bFile = false;
		bool bTime = false;
		if (worldObjectPointer != NULL)
		{
			char key[512];
			worldObjectPointer->SetID(ID);
			for(unsigned int i = 1; i < msgArray.size(); i++)
			{
				sscanf(msgArray[i].c_str(), "%s ", key);

				if(strcmp(key, POSITION.c_str()) == 0)
				{
					position = this->zMsgHandler.ConvertStringToVector(POSITION, msgArray[i]);
					
					bPos = true;
				}
				else if(strcmp(key, ROTATION.c_str()) == 0)
				{
					rotation = this->zMsgHandler.ConvertStringToQuaternion(ROTATION, msgArray[i]);

					bRot = true;
				}
				else if(strcmp(key, STATE.c_str()) == 0)
				{
					state = this->zMsgHandler.ConvertStringToInt(STATE, msgArray[i]);

					bState = true;
				}
				else if(strcmp(key, FRAME_TIME.c_str()) == 0)
				{
					bTime = true;
					serverTime = this->zMsgHandler.ConvertStringToFloat(FRAME_TIME, msgArray[i]);
				}
				else if(strcmp(key, MESH_MODEL.c_str()) == 0)
				{
					bFile = true;
					filename = this->zMsgHandler.ConvertStringToSubstring(MESH_MODEL, msgArray[i]);
				}
				else
				{
					MaloW::Debug("C: Unknown Message Was sent from server -" + msgArray[i] + "- in HandleUpdateObject");
				}
			}
			if (bPos)
			{
				//if (bTime)
				//{
					//float maxLimit = this->zFrameTime + 5;
					//float minLimit = this->zFrameTime - 5;
					//if (serverTime < maxLimit && serverTime > minLimit)
					//{
						switch (objectType)
						{
						case PLAYER:
							dynamic_cast<Player*>(worldObjectPointer)->SetNextPosition(position);
							break;
						case ANIMAL:
							dynamic_cast<Animal*>(worldObjectPointer)->SetNextPosition(position);
							break;
						case STATIC_OBJECT:
							worldObjectPointer->SetObjectPosition(position);
							break;
						default:
							break;
						}
					//}
				//}
			}
			if (bRot)
			{
				worldObjectPointer->SetObjectRotation(rotation);
			}
			if (bState)
			{
				switch (objectType)
				{
				case PLAYER:
					dynamic_cast<Player*>(worldObjectPointer)->SetPlayerState(state);
					break;
				case ANIMAL:
					dynamic_cast<Animal*>(worldObjectPointer)->SetAnimalState(state);
					break;
				default:
					break;
				}
			}
			if (bFile)
			{
				//Create a new Mesh with the current values
				iMesh* mesh = this->zEng->CreateStaticMesh(filename.c_str(), worldObjectPointer->GetObjectPosition());
				float scale = worldObjectPointer->GetObjectScale().y;
				Vector4 quat = worldObjectPointer->GetObjectRotation();

				mesh->Scale(scale);
				mesh->SetQuaternion(Vector4(quat.x, quat.y, quat.z, quat.w));
				if (worldObjectPointer->HasMesh())
				{
					this->zEng->DeleteMesh(worldObjectPointer->GetObjectMesh());
				}
				worldObjectPointer->AddStaticMesh(mesh);
			}

			//Copy over new Values to the vector
			switch (objectType)
			{
			case PLAYER:
				this->zPlayers[pos] = dynamic_cast<Player*>(worldObjectPointer);
				break;
			case ANIMAL:
				this->zAnimals[pos] = dynamic_cast<Animal*>(worldObjectPointer);
				break;
			case STATIC_OBJECT:
				this->zStaticObjects[pos] = dynamic_cast<StaticObject*>(worldObjectPointer);
				break;
			default:
				break;
			}
		}
	}
}