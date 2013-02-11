#include "Game.h"
#include "GameModeFFA.h"
#include "Behavior.h"
#include <world/World.h>
#include "GameEvents.h"
#include "PlayerHumanBehavior.h"
#include "ProjectileBehavior.h"
#include "PlayerWolfBehavior.h"
#include "PlayerActor.h"
#include "ProjectileActor.h"
#include "DeerActor.h"
#include "WolfActor.h"
#include "BearActor.h"
#include "ItemActor.h"
#include "PlayerWolfBehavior.h"
#include "AIWolfBehavior.h"
#include "WorldActor.h"
#include "ItemActor.h"
#include <World/EntityList.h>
#include "Physics.h"
#include "ClientServerMessages.h"

Game::Game(PhysicsEngine* phys, ActorSynchronizer* syncher, std::string mode, const std::string& worldFile ) :
	zPhysicsEngine(phys)
{
	if (mode.find("FFA") == 0 )
	{
		zGameMode = new GameModeFFA(this, 10);
	}
	else
	{
		zGameMode = new GameModeFFA(this, 10);
	}

	// Load Entities
	LoadEntList("Entities.txt");

	// Game Mode Observes
	AddObserver(zGameMode);

	// Create World
	if(worldFile != "")
		zWorld = new World(this, worldFile.c_str());
	else
		zWorld = new World(this, 10, 10);  // Handle Error.

	// Actor Manager
	zActorManager = new ActorManager(syncher);
}

Game::~Game()
{	
	for( auto i = zBehaviors.begin(); i != zBehaviors.end(); ++i )
	{
		delete *i;
	}
	zBehaviors.clear();

	for( auto i = zPlayers.begin(); i != zPlayers.end(); ++i )
	{
		delete i->second;
	}
	zPlayers.clear();

	if ( zGameMode ) delete zGameMode;
	if ( zWorld ) delete zWorld;
	if ( zActorManager ) delete zActorManager;
}

bool Game::Update( float dt )
{
	// Update Behaviors
	auto i = zBehaviors.begin();
	while( i != zBehaviors.end() )
	{
		if ( (*i)->Update(dt) )
		{
			i = zBehaviors.erase(i);
		}
		else
		{
			++i;
		}
	}

	// Update Game Mode, Might Notify That GameMode is Finished
	if ( !zGameMode->Update(dt) )
		return false;
	
	// Update World
	zWorld->Update();

	// Game Still Active
	return true;
}

void Game::OnEvent( Event* e )
{
	// TODO: Incoming Message

	if ( PlayerConnectedEvent* PCE = dynamic_cast<PlayerConnectedEvent*>(e) )
	{
		// Create Player
		Player* player = new Player(PCE->clientData);
		zPlayers[PCE->clientData] = player;
		
		//Lets gamemode observe players.
		player->AddObserver(this->zGameMode);

		//Tells the client it has been connected.
		NetworkMessageConverter NMC;
		std::string message;

		message = NMC.Convert(MESSAGE_TYPE_CONNECTED);
		PCE->clientData->Send(message);

		// Sends the world name
		message = NMC.Convert(MESSAGE_TYPE_LOAD_MAP, zWorld->GetFileName());
		PCE->clientData->Send(message);

		// Send event to game so it knows what players there are.
		// PlayerAddEvent* PAE = new PlayerAddEvent();
		// PAE->player = player;
		// NotifyObservers(PAE);
	}
	else if( KeyDownEvent* KDE = dynamic_cast<KeyDownEvent*>(e) )
	{
		zPlayers[KDE->clientData]->GetKeys().SetKeyState(KDE->key, true);
	}
	else if( KeyUpEvent* KUE = dynamic_cast<KeyUpEvent*>(e) )
	{
		zPlayers[KUE->clientData]->GetKeys().SetKeyState(KUE->key, false);
	}
	else if( ClientDataEvent* CDE = dynamic_cast<ClientDataEvent*>(e) )
	{
		if( PlayerBehavior* dCastBehavior = dynamic_cast<PlayerBehavior*>(zPlayers[CDE->clientData]->GetBehavior()))
			dCastBehavior->ProcessClientData(CDE->direction, CDE->rotation);
	}
	else if ( PlayerDisconnectedEvent* PDCE = dynamic_cast<PlayerDisconnectedEvent*>(e) )
	{
		// Delete Player Behavior
		auto playerIterator = zPlayers.find(PDCE->clientData);
		auto playerBehavior = playerIterator->second->GetBehavior();

		// Create AI Behavior For Players That Disconnected
		if ( PlayerWolfBehavior* playerWolf = dynamic_cast<PlayerWolfBehavior*>(playerBehavior) )
		{
			AIWolfBehavior* aiWolf = new AIWolfBehavior(playerWolf->GetActor(), zWorld);
			SetPlayerBehavior(playerIterator->second, 0);
			zBehaviors.insert(aiWolf);
		}

		// Delete Player and notify GameMode
		auto i = zPlayers.find(PDCE->clientData);

		/*PlayerRemoveEvent* PRE = new PlayerRemoveEvent();
		PRE->player = i->second;
		NotifyObservers(PRE);*/

		delete i->second;
		zPlayers.erase(i);
	}
	else if(PlayerLootObjectEvent* PLOE = dynamic_cast<PlayerLootObjectEvent*>(e))
	{
		std::set<Actor*> actors = this->zActorManager->GetActors();
		std::vector<Item*> lootedItems;

		auto playerIterator = this->zPlayers.find(PLOE->clientData);
		auto playerBehavior = playerIterator->second->GetBehavior();
		Actor* actor = playerBehavior->GetActor();
		for (auto it_actor = actors.begin(); it_actor != actors.end(); it_actor++)
		{
			for (auto it_ID = PLOE->actorID.begin(); it_ID != PLOE->actorID.end(); it_ID++)
			{
				if ((*it_ID) == (*it_actor)->GetID())
				{
					if ((actor->GetPosition() - (*it_actor)->GetPosition()).GetLength() > 4.0f)
						continue;
					
					if (ItemActor* iActor = dynamic_cast<ItemActor*>(*it_actor))
					{
						lootedItems.push_back(iActor->GetItem());
					}
					else if(AnimalActor* aActor = dynamic_cast<AnimalActor*>(*it_actor))
					{
						if (PlayerActor* player = dynamic_cast<PlayerActor*>(actor))
						{
							if (!aActor->IsAlive())
							{
								if (Inventory* playerInventory = player->GetInventory())
								{
									Item* item = playerInventory->SearchAndGetItemFromType(ITEM_TYPE_WEAPON_MELEE, ITEM_SUB_TYPE_POCKET_KNIFE);

									if (item)
									{
										Inventory* inv = aActor->GetInventory();

										std::vector<Item*> items = inv->GetItems();
										for (auto it_Item = items.begin(); it_Item != items.end(); it_Item++)
										{
											lootedItems.push_back((*it_Item));
										}
									}
								}
							}
						}
					}
					else if(PlayerActor* pActor = dynamic_cast<PlayerActor*>(*it_actor))
					{
						if (!pActor->IsAlive())
						{
							Inventory* inv = pActor->GetInventory();

							std::vector<Item*> items = inv->GetItems();
							for (auto it_Item = items.begin(); it_Item != items.end(); it_Item++)
							{
								lootedItems.push_back((*it_Item));
							}
							Item* item = inv->GetRangedWeapon();
							if (item)
							{
								lootedItems.push_back(item);
							}
							item = inv->GetMeleeWeapon();
							if (item)
							{
								lootedItems.push_back(item);
							}
							item = inv->GetProjectile();
							if (item)
							{
								lootedItems.push_back(item);
							}
						}
					}
				}
			}
		}
		if (lootedItems.size() > 0)
		{
			NetworkMessageConverter NMC;
			std::string msg = NMC.Convert(MESSAGE_TYPE_LOOT_OBJECT_RESPONSE);
			for (auto x = lootedItems.begin(); x < lootedItems.end(); x++)
			{
				msg += (*x)->ToMessageString(&NMC); 	
			}
			PLOE->clientData->Send(msg);
		}
	}
	else if ( PlayerPickupObjectEvent* PPOE = dynamic_cast<PlayerPickupObjectEvent*>(e) )
	{
		
	}
	else if ( PlayerDropItemEvent* PDIE = dynamic_cast<PlayerDropItemEvent*>(e) )
	{
		Actor* actor  = NULL;
		PlayerActor* pActor = NULL;
		Item* item = NULL;

		auto i = this->zPlayers.find(PDIE->clientData);

		actor = i->second->GetBehavior()->GetActor();
		pActor = dynamic_cast<PlayerActor*>(actor);

		if(!pActor)
			return;
		
		item = pActor->DropItem(PDIE->itemID);
		
		if(!item)
			return;
 
		actor = NULL;
		actor = new ItemActor(item);
		actor->SetPosition(pActor->GetPosition());
		this->zActorManager->AddActor(actor);

	}
	else if ( PlayerUseItemEvent* PUIE = dynamic_cast<PlayerUseItemEvent*>(e) )
	{
		
	}
	else if ( PlayerUseEquippedWeaponEvent* PUEWE = dynamic_cast<PlayerUseEquippedWeaponEvent*>(e) )
	{
		Actor* actor = NULL;
		PlayerActor* pActor = NULL;
		Inventory* inventory = NULL;
		Item* item = NULL;
		auto playerIterator = zPlayers.find(PUEWE->clientData);

		actor = playerIterator->second->GetBehavior()->GetActor();
		
		if (!(pActor = dynamic_cast<PlayerActor*>(actor)))
		{
			MaloW::Debug("Actor cannot be found in Game.cpp, onEvent, PlayerUseEquippedWeaponEvent.");
			return;
		}
		if(!(inventory = pActor->GetInventory()))
		{
			MaloW::Debug("Inventory is null in Game.cpp, onEvent, PlayerUseEquippedWeaponEvent.");
			return;
		}
		if (!(item = inventory->SearchAndGetItem(PUEWE->itemID)))
		{
			MaloW::Debug("Item is null in Game.cpp, onEvent, PlayerUseEquippedWeaponEvent.");
			return;
		}

		RangedWeapon* ranged = NULL;
		MeleeWeapon* meele = NULL;
		Projectile* proj = NULL;
		NetworkMessageConverter NMC;

		if(ranged = dynamic_cast<RangedWeapon*>(item))
		{
			//Check if arrows are equipped
			Projectile* arrow = inventory->GetProjectile();
			if(arrow->GetItemSubType() == ITEM_SUB_TYPE_ARROW)
			{
				//create projectileActor
				PhysicsObject* pObj = this->zPhysicsEngine->CreatePhysicsObject(arrow->GetModel());
				ProjectileActor* projActor = new ProjectileActor(pActor, pObj);
				ProjectileBehavior* projBehavior= new ProjectileBehavior(projActor, this->zWorld);
				Damage damage;

				//Sets damage
				damage.piercing = ranged->GetDamage() + arrow->GetDamage();
				projActor->SetDamage(damage);

				this->zActorManager->AddActor(projActor);
				this->zBehaviors.insert(projBehavior);
				//Decrease stack
				arrow->Use();
				if (arrow->GetStackSize() <= 0)
				{
					std::string msg = NMC.Convert(MESSAGE_TYPE_REMOVE_EQUIPMENT, (float)arrow->GetID());
					msg += NMC.Convert(MESSAGE_TYPE_EQUIPMENT_SLOT, (float)EQUIPMENT_SLOT_AMMO);
					PUEWE->clientData->Send(msg);
					SAFE_DELETE(arrow);
					inventory->UnEquipProjectile();
				}
				//Send feedback message
				PUEWE->clientData->Send(NMC.Convert(MESSAGE_TYPE_WEAPON_USE, (float)pActor->GetID()));
			}
		}
		else if(proj = dynamic_cast<Projectile*>(item))
		{
			//TODO: Implement rocks
		}
		else if(meele = dynamic_cast<MeleeWeapon*>(item))
		{
			float range = 0.f; 
			BioActor* victim = NULL;

			//Check Collisions
			range = meele->GetRange();
			victim = dynamic_cast<BioActor* >(this->zActorManager->CheckCollisions(pActor, range));

			if(victim)
			{
				Damage dmg;

				if(meele->GetItemSubType() == ITEM_SUB_TYPE_MACHETE)
					dmg.slashing = meele->GetDamage();
				
				victim->TakeDamage(dmg, pActor);
			}
		}

	}
	else if (PlayerEquipItemEvent* PEIE = dynamic_cast<PlayerEquipItemEvent*>(e) )
	{

	}
	else if (PlayerUnEquipItemEvent* PUEIE = dynamic_cast<PlayerUnEquipItemEvent*>(e) )
	{

	}
	else if ( EntityUpdatedEvent* EUE = dynamic_cast<EntityUpdatedEvent*>(e) )
	{
		//auto i = zWorldActors.find(EUE->entity);
		//if ( i != zWorldActors.end() )
		//{
		//	i->second->SetPosition(EUE->entity->GetPosition());
		//	// TODO: Rotation
		//	i->second->SetScale(EUE->entity->GetScale());
		//}
	}
	else if ( EntityLoadedEvent* ELE = dynamic_cast<EntityLoadedEvent*>(e) )
	{
		//PhysicsObject* phys = 0;
		//
		//if ( GetEntBlockRadius(ELE->entity->GetType()) > 0.0f )
		//{
		//	phys = zPhysicsEngine->CreatePhysicsObject(GetEntModel(ELE->entity->GetType()), ELE->entity->GetPosition());
		//}

		//// Create Physics Object
		//WorldActor* actor = new WorldActor();
		//zWorldActors[ELE->entity] = actor;
		//zActorManager->AddActor(actor);

		//actor->AddObserver(this->zGameMode);
	}
	else if ( EntityRemovedEvent* ERE = dynamic_cast<EntityRemovedEvent*>(e) )
	{
		auto i = zWorldActors.find(ERE->entity);
		if ( i != zWorldActors.end() )
		{
			delete i->second;
			zWorldActors.erase(i);
		}
	}
	else if ( UserDataEvent* UDE = dynamic_cast<UserDataEvent*>(e) )
	{
		// Create Player Actor
		PhysicsObject* pObject = 0; //this->zPhysicsEngine->CreatePhysicsObject(UDE->playerModel);
		Actor* actor = new PlayerActor(zPlayers[UDE->clientData], pObject);
		zActorManager->AddActor(actor);
		actor->AddObserver(this->zGameMode);

		// Start Position
		Vector3 center;
		center.x = zWorld->GetWorldCenter().x;
		center.z = zWorld->GetWorldCenter().y;
		
		center = this->CalcPlayerSpawnPoint(32, center.GetXZ());
		actor->SetPosition(center);

		// Apply Default Player Behavior
		SetPlayerBehavior(zPlayers[UDE->clientData], new PlayerHumanBehavior(actor, zWorld, zPlayers[UDE->clientData]));

		//Tells the client which Actor he owns.
		std::string message;
		NetworkMessageConverter NMC;
		unsigned int selfID;

		selfID = actor->GetID();
		message = NMC.Convert(MESSAGE_TYPE_SELF_ID, (float)selfID);
		UDE->clientData->Send(message);

		//Gather Actors Information and send to client
		std::set<Actor*>& actors = this->zActorManager->GetActors();
		for (auto it = actors.begin(); it != actors.end(); it++)
		{
			message =  NMC.Convert(MESSAGE_TYPE_NEW_ACTOR, (float)(*it)->GetID());
			message += NMC.Convert(MESSAGE_TYPE_POSITION, (*it)->GetPosition());
			message += NMC.Convert(MESSAGE_TYPE_ROTATION, (*it)->GetRotation());
			message += NMC.Convert(MESSAGE_TYPE_SCALE, (*it)->GetScale());
			message += NMC.Convert(MESSAGE_TYPE_MESH_MODEL, (*it)->GetModel());

			//Sends this Actor to the new player
			UDE->clientData->Send(message);
		}

	}
	else if ( WorldLoadedEvent* WLE = dynamic_cast<WorldLoadedEvent*>(e) )
	{

	}

	NotifyObservers(e);
}

void Game::SetPlayerBehavior( Player* player, PlayerBehavior* behavior )
{
	// Find Old Behavior
	Behavior* curPlayerBehavior = player->GetBehavior();

	// Find In Behaviors
	if ( curPlayerBehavior )
	{
		zBehaviors.erase(curPlayerBehavior);
		delete curPlayerBehavior;
	}

	// Set New Behavior
	if ( behavior )	zBehaviors.insert(behavior);
	player->zBehavior = behavior;
}

Vector3 Game::CalcPlayerSpawnPoint(int maxPoints, Vector2 center)
{
	int point = this->zPlayers.size();

	static const float PI = 3.14159265358979323846f;
	static const float radius = 20.0f;
	float slice  = 2 * PI / maxPoints;

	float angle = slice * point;

	float x = center.x + radius * cos(angle);
	float z = center.y + radius * sin(angle);
	float y = 0.0f;

	if ( x >= 0.0f && z >= 0.0f && x < zWorld->GetWorldSize().x && z < zWorld->GetWorldSize().y )
	{
		y = this->zWorld->CalcHeightAtWorldPos(Vector2(x, z));
	}

	return Vector3(x, y, z);
}

Vector3 Game::CalcPlayerSpawnPoint(int nr)
{
	int point = nr;

	static const float PI = 3.14159265358979323846f;
	static const float radius = 20.0f;
	float slice  = 2 * PI / this->zMaxNrOfPlayers;

	float angle = slice * point;

	float x = zWorld->GetWorldCenter().x + radius * cos(angle);
	float z = zWorld->GetWorldCenter().y + radius * sin(angle);
	float y = 0.0f;

	if ( x >= 0.0f && z >= 0.0f && x < zWorld->GetWorldSize().x && z < zWorld->GetWorldSize().y )
	{
		y = this->zWorld->CalcHeightAtWorldPos(Vector2(x, z));
	}

	return Vector3(x, y, z);
}
