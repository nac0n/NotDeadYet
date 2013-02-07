#include "Container.h"

Container::Container(const unsigned int ID, const unsigned int itemType, const unsigned int itemSubType,
					 const int maxUse, const int currUse) : Item(ID, itemType, itemSubType)
{
	this->zStacking = false;
	this->zMaxUses = maxUse;
	this->zUsesRemaining = currUse;
}

Container::Container(const Container& other)
{
	this->zID = other.zID;
	this->zStacks = other.zStacks;
	this->zWeight = other.zWeight;
	this->zMaxUses = other.zMaxUses;
	this->zItemName = other.zItemName;
	this->zIconPath = other.zIconPath;
	this->zItemType = other.zItemType;
	this->zItemSubType = other.zItemSubType;
	this->zUsesRemaining = other.zUsesRemaining;
	this->zItemDescription = other.zItemDescription;
}

Container::Container(const Container* other)
{
	this->zID = other->zID;
	this->zStacks = other->zStacks;
	this->zWeight = other->zWeight;
	this->zMaxUses = other->zMaxUses;
	this->zItemName = other->zItemName;
	this->zIconPath = other->zIconPath;
	this->zItemType = other->zItemType;
	this->zItemSubType = other->zItemSubType;
	this->zUsesRemaining = other->zUsesRemaining;
	this->zItemDescription = other->zItemDescription;
}

Container::~Container()
{

}

bool Container::Use()
{
	if (this->zUsesRemaining > 0)
	{
		this->zUsesRemaining -= 1;
		return true;
	}
	return false;
}

std::string Container::ToMessageString( NetworkMessageConverter* NMC )
{
	std::string msg = Item::ToMessageString(NMC);

	msg += NMC->Convert(MESSAGE_TYPE_CONTAINER_MAX, (float)this->zMaxUses);
	msg += NMC->Convert(MESSAGE_TYPE_CONTAINER_CURRENT, (float)this->zUsesRemaining);
	
	return msg;
}