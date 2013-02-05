#include "StaticProjectileObject.h"


StaticProjectileObject::StaticProjectileObject( const bool genID /*= true*/ ) : StaticObjectActor(genID)
{
	this->zSpeed = 1.0f;
	this->zStacks = 1;
	this->zWeight = 1;
	this->zActorType = ACTOR_TYPE_STATIC_OBJECT_PROJECTILE;
}


StaticProjectileObject::StaticProjectileObject(const StaticProjectileObject& other, bool genID)
{
	if (genID)
		this->zID = this->GenerateID();
	else
		this->zID = other.GetID();

	this->zRot = other.zRot;
	this->zPos = other.zPos;
	this->zType = other.zType;
	this->zScale = other.zScale;
	this->zSpeed = other.zSpeed;
	this->zStacks = other.zStacks;
	this->zDamage = other.zDamage;
	this->zWeight = other.zWeight;
	this->zSubType = other.zSubType;
	this->zIconPath = other.zIconPath;
	this->zActorType = other.zActorType;
	this->zActorModel = other.zActorModel;
	this->zDescription = other.zDescription;
	this->zActorObjectName = other.zActorObjectName;
	
}

StaticProjectileObject::StaticProjectileObject(const DynamicProjectileObject* other, bool genID)
{

	if (genID)
		this->zID = this->GenerateID();
	else
		this->zID = other->GetID();

	this->zType = other->GetType();
	this->zSpeed = other->GetSpeed();
	this->zScale = other->GetScale();
	this->zRot = other->GetRotation();
	this->zPos = other->GetPosition();
	this->zDamage = other->GetDamage();
	this->zWeight = other->GetWeight();
	this->zSubType = other->GetSubType();
	this->zStacks = other->GetStackSize();
	this->zIconPath = other->GetIconPath();
	this->zActorModel = other->GetActorModel();
	this->zDescription = other->GetDescription();
	this->zActorObjectName = other->GetActorObjectName();
	this->zActorType = ACTOR_TYPE_STATIC_OBJECT_PROJECTILE;
}

StaticProjectileObject::StaticProjectileObject( const StaticProjectileObject* other, bool genID /*= false*/ )
{
	if (genID)
		this->zID = this->GenerateID();
	else
		this->zID = other->GetID();

	this->zRot = other->zRot;
	this->zPos = other->zPos;
	this->zType = other->zType;
	this->zSpeed = other->zSpeed;
	this->zScale = other->zScale;
	this->zStacks = other->zStacks;
	this->zDamage = other->zDamage;
	this->zWeight = other->zWeight;
	this->zSubType = other->zSubType;
	this->zIconPath = other->zIconPath;
	this->zActorType = other->zActorType;
	this->zActorModel = other->zActorModel;
	this->zDescription = other->zDescription;
	this->zActorObjectName = other->zActorObjectName;
}

StaticProjectileObject::~StaticProjectileObject()
{

}