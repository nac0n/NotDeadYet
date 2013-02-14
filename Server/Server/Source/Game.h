#pragma once

#include <Observer.h>
#include <map>
#include <KeyStates.h>
#include "ActorManager.h"
#include "ActorSynchronizer.h"
#include "PhysicsEngine.h"

class ClientData;
class World;
class GameMode;
class Player;
class Behavior;
class PlayerBehavior;
class ActorSynchronizer;
class KeyStates;
class Entity;
class WorldActor;
class ItemActor;

class Game : public Observer, public Observed
{
	ActorManager* zActorManager;
	GameMode* zGameMode;
	World* zWorld;

	std::map<ClientData*, Player*> zPlayers;
	std::map<Entity*, WorldActor*> zWorldActors;

	std::set<Behavior*> zBehaviors;

	int zMaxNrOfPlayers;

public:
	Game(PhysicsEngine* phys, ActorSynchronizer* syncher, std::string mode, const std::string& worldFile);
	virtual ~Game();

	// Returns false if game has finished
	bool Update( float dt );

	// Event input
	void OnEvent( Event* e );

	// Sets The Behavior Of Player
	void SetPlayerBehavior( Player* player, PlayerBehavior* behavior );
	/*! Converts an item to ItemActor*/
	ItemActor* ConvertToItemActor(Behavior* behavior,  Actor*& oldActorOut);
	
	Vector3 CalcPlayerSpawnPoint(int MaxPoints, Vector2 center);
	Vector3 CalcPlayerSpawnPoint(int nr);
	
	ActorManager* GetActorManager() {return this->zActorManager;}
	World* GetWorld() {return this->zWorld;}
private:
	//Test function, spawns items
	void SpawnItemsDebug();
private:
	PhysicsEngine* zPhysicsEngine;
};