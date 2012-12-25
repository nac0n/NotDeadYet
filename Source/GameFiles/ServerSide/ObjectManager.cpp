#include "ObjectManager.h"
#include "MaloWLib/MaloW.h"
#include <fstream>
#include <string>
#include <algorithm>

static const std::string FILENAME		=	"WorldObjects.cfg";
static const std::string OBJECT_WEAPON	=	"[Object.Weapon]";
static const std::string OBJECT_FOOD	=	"[Object.Food]";

static const std::string END			=	"#END";
static const std::string TYPE			=	"TYPE";
static const std::string WEIGHT			=	"WEIGHT";
static const std::string HUNGER			=	"HUNGER";
static const std::string DAMAGE			=	"DAMAGE";
static const std::string MODEL			=	"MODEL";
static const std::string NAME			=	"NAME";
static const std::string RANGE			=	"RANGE";
static const std::string IS_RANGED		=	"IS_RANGED";


ObjectManager::ObjectManager()
{

}

ObjectManager::~ObjectManager()
{
	auto x = zWeapons.begin();
	auto i = zFood.begin();

	for(; x < zWeapons.end(); x++)
	{
		SAFE_DELETE((*x));
	}

	for(; i < zFood.end(); i++)
	{
		SAFE_DELETE((*i));
	}
}

bool ObjectManager::ReadObjects()
{
	std::ifstream read;
	read.open(FILENAME);

	if(!read.is_open())
		return false;

	while(!read.eof())
	{
		char line[256];
		char key[52];
		char command[126];

		read.getline(line,sizeof(line));

		if(strcmp(line, "") == 0)
			continue;

		TrimAndSet(line);

		sscanf_s(line, "%s", &key, sizeof(key));

		if(strcmp(key, OBJECT_WEAPON.c_str()) == 0)
		{
			WeaponObject *wp = new WeaponObject(false);
			while(!read.eof() && strcmp(command, END.c_str()) != 0)
			{
				read.getline(line, sizeof(line));
				TrimAndSet(line);

				sscanf_s(line, "%s = %s" , &command, sizeof(command), &key, sizeof(key));

				if(strcmp(command,END.c_str()) != 0)
					InterpCommand(command, key, wp);
			}
			this->zWeapons.push_back(wp);
		}

		else if(strcmp(key, OBJECT_FOOD.c_str()) == 0)
		{
			FoodObject *fd = new FoodObject(false);
			while(!read.eof() && strcmp(key, END.c_str()) != 0)
			{
				read.getline(line, sizeof(line));
				TrimAndSet(line);

				sscanf_s(line, "%s = %s" , &command, sizeof(command), &key, sizeof(key));

				if(strcmp(command,END.c_str()) != 0)
					InterpCommand(command, key, fd);
			}

			this->zFood.push_back(fd);
		}


	}

	return true;
}

void ObjectManager::TrimAndSet( char* ret )
{
	if(ret == NULL)
		return;

	std::string str(ret);
	str.erase(remove_if(str.begin(), str.end(), isspace), str.end());

	size_t found;
	found = str.find("=");

	if(found == std::string::npos)
		return;

	int index = (int)found;

	std::string subA, subB;
	subA = str.substr(0,index);
	subB = str.substr(index+1,str.size()-1);

	subA += " = ";
	str = subA + subB;

	strcpy(ret, str.c_str());

}

bool ObjectManager::InterpCommand( char* command, char* key, WeaponObject* wp )
{
	if(strcmp(key, "") == 0)
		return false;

	string CC(command);
	std::transform(CC.begin(), CC.end(), CC.begin(), ::toupper);
	strcpy(command, CC.c_str());

	if(strcmp(command, TYPE.c_str()) == 0)
	{
		wp->SetType(MaloW::convertStringToInt(key));
	}
	else if(strcmp(command, DAMAGE.c_str()) == 0)
	{
		wp->SetDamage(MaloW::convertStringToFloat(key));
	}
	else if(strcmp(command, WEIGHT.c_str()) == 0)
	{
		wp->SetWeight(MaloW::convertStringToInt(key));
	}
	else if(strcmp(command, RANGE.c_str()) == 0)
	{
		wp->SetRange(MaloW::convertStringToFloat(key));
	}
	else if(strcmp(command, MODEL.c_str()) == 0)
	{
		wp->SetActorModel(key);
	}
	else if(strcmp(command, NAME.c_str()) == 0)
	{
		wp->SetActorObjectName(key);
	}

	return true;
}

bool ObjectManager::InterpCommand( char* command, char* key, FoodObject* fd )
{
	if(strcmp(key, "") == 0)
		return false;

	string CC(command);
	std::transform(CC.begin(), CC.end(), CC.begin(), ::toupper);
	strcpy(command, CC.c_str());

	if(strcmp(command, TYPE.c_str()) == 0)
	{
		fd->SetType(MaloW::convertStringToInt(key));
	}
	else if(strcmp(command, HUNGER.c_str()) == 0)
	{
		fd->SetHunger(MaloW::convertStringToFloat(key));
	}
	else if(strcmp(command, WEIGHT.c_str()) == 0)
	{
		fd->SetWeight(MaloW::convertStringToInt(key));
	}
	else if(strcmp(command, MODEL.c_str()) == 0)
	{
		fd->SetActorModel(key);
	}
	else if(strcmp(command, NAME.c_str()) == 0)
	{
		fd->SetActorObjectName(key);
	}


	return true;
}

const WeaponObject* ObjectManager::GetWeaponObject( const int type )
{
	return SearchType(zWeapons, type);
}

const FoodObject* ObjectManager::GetFoodObject( const int type )
{
	return SearchType(zFood, type);
}

const WeaponObject* ObjectManager::SearchType( std::vector<WeaponObject*>& weapons, const int type ) const
{
	for(auto it = weapons.begin(); it < weapons.end(); it++)
	{
		if((*it)->GetType() == type)
			return (*it);
	}

	return NULL;
}

const FoodObject* ObjectManager::SearchType( std::vector<FoodObject*>& food, const int type ) const
{
	for(auto it = food.begin(); it < food.end(); it++)
	{
		if((*it)->GetType() == type)
			return (*it);
	}

	return NULL;
}
