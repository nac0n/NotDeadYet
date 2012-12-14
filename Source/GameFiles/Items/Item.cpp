#include "GameFiles/Items/Item.h"


Item::Item()
{
	this->InitValues();
}

Item::Item( const int id )
{
	this->InitValues(id);
}

Item::Item(const unsigned int id, const unsigned int weight, const std::string& name)
{
	this->InitValues(id, weight, name);
}

void Item::InitValues(unsigned int id, unsigned int weight, const std::string& name)
{

}
