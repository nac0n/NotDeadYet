#include "Behavior.h"

Behavior::Behavior( Actor* actor, World* world)
{
	this->zActor = actor;
	this->zWorld = world;
	this->zAnchor = zWorld->CreateAnchor();
}