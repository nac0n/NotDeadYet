/*
	Made by Jensen Christopher Datum(11/12/12 created) 
	for project Not Dead Yet at Blekinge tekniska h�gskola
*/

#pragma once

#include "Weapon.h"

class MeleeWeapon : public Weapon
{
public:
	MeleeWeapon();
	MeleeWeapon(const unsigned int ID, const unsigned int itemType, const int itemSubType, const float damage, const float range);
	MeleeWeapon(const unsigned int itemType, const int itemSubType, const float damage, const float range);
	MeleeWeapon(const MeleeWeapon& other);
	virtual ~MeleeWeapon();

	virtual bool Use();
	void UseWeapon(float& range, float& damage);
private:
	
};