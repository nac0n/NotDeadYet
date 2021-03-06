#include <World/World.h>
#include "PlayerGhostBehavior.h"
#include "GhostActor.h"
#include "Player.h"
#include "Actor.h"

static const float MAX_VELOCITY = 7.0f;
static const float ACCELERATION = 5.0f;

PlayerGhostBehavior::PlayerGhostBehavior( Actor* actor, World* world, Player* player ) : PlayerBehavior(actor, world, player)
{

}

PlayerGhostBehavior::~PlayerGhostBehavior()
{

}

bool PlayerGhostBehavior::Update( float dt )
{
	if ( !zPlayer )
		return true;

	if ( PlayerBehavior::Update(dt) )
		return true;

	KeyStates keyStates = this->zPlayer->GetKeys();
	Vector3 newPlayerPos;
	Vector3 moveDir = Vector3(0.0f, 0.0f, 0.0f);

	Vector3 currentPlayerPos = this->zActor->GetPosition();

	//Get Directions
	Vector3 currentPlayerDir = this->zActor->GetDir();
	Vector3 currentPlayerUp = Vector3(0.0f, 1.0f, 0.0f);
	currentPlayerDir.Normalize();
	Vector3 currentPlayerRight = currentPlayerUp.GetCrossProduct(currentPlayerDir);
	currentPlayerRight.Normalize();
	try
	{
		Vector3 currentGroundNormal = this->zWorld->CalcNormalAt(currentPlayerPos.GetXZ());
	}
	catch(...)
	{

	}
	
	// Calc the movement vector
	moveDir += currentPlayerDir * (float)(keyStates.GetKeyState(KEY_FORWARD) - //if KEY_BACKWARD then currentPlayerDir inverse 
		keyStates.GetKeyState(KEY_BACKWARD));

	moveDir += currentPlayerRight * (float)(keyStates.GetKeyState(KEY_RIGHT) - keyStates.GetKeyState(KEY_LEFT));

	moveDir += currentPlayerUp * (float)(keyStates.GetKeyState(KEY_JUMP) - keyStates.GetKeyState(KEY_DUCK));

	moveDir *= 10.0f;

	if(moveDir == Vector3(0.0f, 0.0f, 0.0f))
	{
		this->zVelocity = Vector3(0.0f, 0.0f, 0.0f);
	}
	else
	{
		this->zVelocity += moveDir * dt;
	}

	

	if(this->zVelocity.GetLength() > MAX_VELOCITY)
	{
		this->zVelocity.Normalize();
		this->zVelocity *= MAX_VELOCITY;
	}

	newPlayerPos = currentPlayerPos + this->zVelocity * dt;

	//Look so Ghost isn't outside of World.
	if ( zWorld->IsInside(newPlayerPos.GetXZ()) )
	{
		float newGroundHeight = 1.0f;
		try
		{
			newGroundHeight = zWorld->CalcHeightAtWorldPos(newPlayerPos.GetXZ()) + 1.0f;
		}
		catch(...)
		{

		}
		if(newGroundHeight > newPlayerPos.y)
		{
			newPlayerPos.y = newGroundHeight;
		}
		else if (newPlayerPos.y > newGroundHeight + 50.0f)
		{
			newPlayerPos.y = newGroundHeight + 50.0f;
		}

		zActor->SetPosition(newPlayerPos);
	}
	else
	{
		Vector2 center = zWorld->GetWorldCenter();
		Vector3 posTemp = Vector3(center.x, 5.0f, center.y);
		zActor->SetPosition(posTemp);
	}

	return false;
}
