#include "Game.h"
#include "time.h"
#include "GameModeFFA.h"
#include "BioActor.h"
#include "DeerActor.h"
#include "WolfActor.h"
#include "BearActor.h"
#include "PlayerActor.h"
#include "GhostActor.h";
#include "GameEvents.h"
#include "PlayerGhostBehavior.h"
#include "PlayerDeerBehavior.h"
#include "PlayerWolfBehavior.h"

GameModeFFA::GameModeFFA(Game* game) : GameMode(game)
{
	srand((unsigned int)time(0));
	//zKillLimit = killLimit;
}

GameModeFFA::~GameModeFFA()
{

}

bool GameModeFFA::Update( float dt )
{
//	for(auto it = zPlayers.begin(); it != zPlayers.end(); it++)
//	{
//		if(zScoreBoard[(*it)] >= zKillLimit)
//		{
//			for(auto i = zPlayers.begin(); i != zPlayers.end(); i++)
//			{
//				MaloW::Debug("Kills: " + MaloW::convertNrToString((float)zScoreBoard[(*i)]));
//			}
//			return false;
//		}
//	}
	return true;
}

void GameModeFFA::OnEvent( Event* e )
{
	if( BioActorTakeDamageEvent* ATD = dynamic_cast<BioActorTakeDamageEvent*>(e) )
	{
		if( PlayerActor* pa = dynamic_cast<PlayerActor*>(ATD->zActor) )
		{
			if(pa->GetHealth() - ATD->zDamage->GetTotal() <= 0)
			{
				// Set new spawn pos
				//int maxPlayers = zPlayers.size();
				//int rand = 1 + (std::rand() % (maxPlayers+1));
				//pa->SetPosition(zGame->CalcPlayerSpawnPoint(rand));
				//pa->SetHealth(pa->GetHealthMax());
				//pa->SetStamina(pa->GetStaminaMax());
				//pa->SetFullness(pa->GetFullnessMax());
				//pa->SetHydration(pa->GetHydrationMax());

				//ATD->zDamage->blunt = 0;
				//ATD->zDamage->fallingDamage = 0;
				//ATD->zDamage->piercing = 0;
				//ATD->zDamage->slashing = 0;

				//Add to scoreboard
				//if( PlayerActor* dpa = dynamic_cast<PlayerActor*>(ATD->zDealer) )
				//	if(dpa != pa)
				//		zScoreBoard[dpa->GetPlayer()]++;
				//	else if( dpa == pa )
				//		zScoreBoard[pa->GetPlayer()]--;

				OnPlayerDeath(pa);
			}
			else
			{
				unsigned int ID = ATD->zDealer->GetID();
				float damage = ATD->zDamage->GetTotal();

				NetworkMessageConverter NMC;
				std::string msg = NMC.Convert(MESSAGE_TYPE_ACTOR_TAKE_DAMAGE, (float)ID);
				msg += NMC.Convert(MESSAGE_TYPE_HEALTH, damage);
				ClientData* cd = pa->GetPlayer()->GetClientData();
				cd->Send(msg);
			}
		}
	}
	else if (PlayerAnimalSwapEvent* PASE = dynamic_cast<PlayerAnimalSwapEvent*>(e))
	{
		GhostActor* gActor = dynamic_cast<GhostActor*>(PASE->zActor);

		if (gActor)
			this->SwapToAnimal(gActor, PASE->zAnimalType);
	}
	else if( PlayerAddEvent* PAE = dynamic_cast<PlayerAddEvent*>(e) )
	{
		//this->zScoreBoard[PAE->player] = 0;
		zPlayers.insert(PAE->player);
	}
	else if( PlayerRemoveEvent* PRE = dynamic_cast<PlayerRemoveEvent*>(e) )
	{
		//this->zScoreBoard.erase(PRE->player);
		this->zPlayers.erase(PRE->player);
	}
}

void GameModeFFA::SwapToAnimal(GhostActor* gActor, unsigned int animalType)
{
	std::set<Actor*> actors = this->zGame->GetActorManager()->GetActors();
	bool found = false;
	Player* player = gActor->GetPlayer();
	ClientData* cd = player->GetClientData();

	NetworkMessageConverter NMC;
	std::string msg = "";
	
	for (auto it_Actors = actors.begin(); it_Actors != actors.end() && !found; it_Actors++)
	{
		if (AnimalActor* aActor = dynamic_cast<AnimalActor*>((*it_Actors)))
		{
			//If type = Deer
			if (animalType == 0)
			{
				if (DeerActor* dActor = dynamic_cast<DeerActor*>(aActor))
				{
					Player* player = gActor->GetPlayer();
					gActor->SetPlayer(NULL);

					
					PlayerDeerBehavior* playerDeerBehavior = new PlayerDeerBehavior(dActor, this->zGame->GetWorld(), player);
					dActor->SetPlayer(player);
					
					this->zGame->RemoveAIBehavior(dActor);
					this->zGame->SetPlayerBehavior(player, playerDeerBehavior);

					msg = NMC.Convert(MESSAGE_TYPE_SELF_ID, dActor->GetID());
					msg += NMC.Convert(MESSAGE_TYPE_ACTOR_TYPE, 3);

					cd->Send(msg);

					this->zGame->GetActorManager()->RemoveActor(gActor);
					found = true;
				}
			}
			/*//If Type = Wolf
			else if (animalType == 1)
			{
				if (WolfActor* wActor = dynamic_cast<WolfActor*>(aActor))
				{
					//Kill player on Client
					Player* player = gActor->GetPlayer();
					gActor->SetPlayer(NULL);

					PlayerWolfBehavior* playerWolfBehavior = new PlayerWolfBehavior(wActor, this->zGame->GetWorld(), player);

					this->zGame->SetPlayerBehavior(player, playerWolfBehavior);

					wActor->SetPlayer(player);

					msg = NMC.Convert(MESSAGE_TYPE_SELF_ID, wActor->GetID());
					msg += NMC.Convert(MESSAGE_TYPE_ACTOR_TYPE, 3);

					cd->Send(msg);

					this->zGame->GetActorManager()->RemoveActor(gActor);
					found = true;
				}
			}*/
			//If Type = Bear
			else if (animalType == 2)
			{
				if (BearActor* dActor = dynamic_cast<BearActor*>(aActor))
				{
					//Player* player = gActor->GetPlayer();
					//gActor->SetPlayer(NULL);

					//PlayerBearBehavior* playerBearBehavior = new PlayerBearBehavior(dActor, this->zGame->GetWorld(), player);
					//dActor->SetPlayer(player);

					//this->zGame->SetPlayerBehavior(player, playerBearBehavior);

					//msg = NMC.Convert(MESSAGE_TYPE_SELF_ID, dActor->GetID());
					//msg += NMC.Convert(MESSAGE_TYPE_ACTOR_TYPE, 3);

					//cd->Send(msg);

					//this->zGame->GetActorManager()->RemoveActor(gActor);
					//found = true;
				}
			}
		}
		

	}
}

void GameModeFFA::OnPlayerDeath(PlayerActor* pActor)
{
	NetworkMessageConverter NMC;
	std::string msg = "";

	Player* player = pActor->GetPlayer();
	pActor->SetPlayer(NULL);

	ClientData* cd = player->GetClientData();

	//Create Spirit
	Vector3 position = pActor->GetPosition();

	Vector3 direction = pActor->GetDir();

	GhostActor* gActor = new GhostActor(player);
	gActor->SetPosition(position);
	gActor->SetDir(direction);
	
	//Create Ghost behavior
	PlayerGhostBehavior* pGhostBehavior = new PlayerGhostBehavior(gActor, this->zGame->GetWorld(), player);

	this->zGame->SetPlayerBehavior(player, pGhostBehavior);

	//Tell Client his new ID and actor type
	ActorManager* aManager = this->zGame->GetActorManager();
	msg = NMC.Convert(MESSAGE_TYPE_SELF_ID, gActor->GetID());
	msg += NMC.Convert(MESSAGE_TYPE_ACTOR_TYPE, 2);

	cd->Send(msg);
	
	//Add the actor to the list
	aManager->AddActor(gActor);
}