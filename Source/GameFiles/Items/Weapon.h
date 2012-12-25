/*
	Made by Jensen Christopher Datum(11/12/12 created) 
	for project Not Dead Yet at Blekinge tekniska h�gskola
*/

#pragma once

#include "GameFiles/Items/Item.h"

 /*! Abstract class for Weapons*/
class Weapon : public Item
{
public:
	Weapon(const unsigned int id,const unsigned int itemType): Item(id, itemType)
	{
		this->zDamage = 0.0f; 
		this->zRange = 0.0f;
	}
	Weapon(const unsigned int id, const unsigned int weight, const std::string& name, const unsigned int itemType,
		const float damage, const float range) : Item(id, weight, name, itemType)
	{
		this->zDamage = damage; 
		this->zRange = range;
	}
	virtual ~Weapon(){}
	/*! Returns The Weapon Damage*/
	float GetDamage() const {return this->zDamage;}
	/*! Returns the Weapon Range*/
	float GetRange() const {return this->zRange;}

	virtual void UseWeapon(float& range, float& damage) = 0;
	virtual void Use() = 0;
protected:
	float zDamage;
	float zRange;
};