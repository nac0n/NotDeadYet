/*
Made by Ed�nge Simon Datum(30/11/12 created) 
for project Not Dead Yet at Blekinge tekniska h�gskola.
*/

#pragma once

#include "BioActor.h"
#include "ObjectManager.h"
#include "KeyValues.h"
#include "KeyStates.h"
#include "Player.h"
#include <World/WorldEvents.h>

/*This class is used to save player information such as position and states.
  This information is sent to clients.
*/
class PlayerActor : public BioActor, public Observed
{
	Vector3 zInitialDirection;

public:
	PlayerActor(Player* player, PhysicsObject* pObj);

	virtual ~PlayerActor();

	/*! Updates players pos, states etc.*/
	void Update(float deltaTime);
	/*! Picks up an object and transform it to an item and adds it to the inventory.
		Genuine actors: StaticObjectActor, DynamicObjectActor.
		This function DO NOT deallocates the Actor object.
	*/

	Item* GetItem(const long ID){return this->zInventory->SearchAndGetItem(ID);}
	float GetLatency() const {return this->zLatency;}
	inline float GetFrameTime() const {return this->zFrameTime;}
	/*! Gets the current key state. This function is used
		to see which buttons are pressed right now.
		Key is an enum defined in header KeyValues.
	*/
	inline bool GetkeyState(const unsigned int key)
	{return zKeyStates.GetKeyState(key);}

	/* ! Sets key states.
		This one is used to define which buttons are being pressed.
		Key is an enum defined in header KeyValues.
	*/
	inline void SetKeyState(const unsigned int key, const bool value)
	{zKeyStates.SetKeyState(key,value);}

	inline void SetFrameTime(const float frameTime){this->zFrameTime = frameTime;}
	inline void SetLatency(const float latency){this->zLatency = latency;}
	
	void SetInventory(Inventory* inv) {this->zInventory = inv;}
	void EatFood(float hunger);
	void Drink(float hydration);
	/*! Returns Pos, Rot, Scale, Stamina, Health, State, Hunger, Hydration.*/
	virtual std::string ToMessageString(NetworkMessageConverter* NMC);
private:
	void InitValues();

private:
	float	zLatency;
	float	zFrameTime;

	float	zHunger;
	float	zHungerMax;
	float	zHydration;
	float	zHydrationMax;

	bool zHydrationChanged;
	bool zHungerChanged;
	
	KeyStates zKeyStates;

	Inventory* zInventory;

	Player* zPlayer;
};

class PlayerUpdatedEvent : public Event
{
public:
	PlayerActor* playerActor;
	bool validMove;
	Vector3 prevPos;
	PlayerUpdatedEvent(PlayerActor* playerActor, bool& validMove, Vector3 prevPos) 
		: playerActor(playerActor), validMove(validMove), prevPos(prevPos){}
};