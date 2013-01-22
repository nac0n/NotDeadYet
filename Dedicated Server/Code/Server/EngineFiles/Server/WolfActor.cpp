#include "WolfActor.h"

WolfActor::WolfActor( bool genID /*= true*/, bool isPlayerControlled /*= false*/  ) : AnimalActor(genID)
{
	this->SetIfPlayerControlled(isPlayerControlled);
	this->InitWolf();
}

WolfActor::WolfActor( const Vector3& startPos, bool genID /*= true*/, bool isPlayerControlled /*= false*/  ) : AnimalActor(startPos, genID)
{
	this->SetIfPlayerControlled(isPlayerControlled);
	this->InitWolf();
}

WolfActor::WolfActor( const Vector3& startPos, const Vector4& rot, bool genID /*= true*/, bool isPlayerControlled /*= false*/  ) : AnimalActor(startPos, rot, genID)
{
	this->SetIfPlayerControlled(isPlayerControlled);
	this->InitWolf();
}

WolfActor::~WolfActor()
{

}

void WolfActor::InitWolf() throw(...)
{
	unsigned int numberFromFile = 0;
	char characters[16];

	std::ifstream infile("wolfVariables.txt");
	if(infile.good()) //Getting values from a textfile needs to be more properly implemented
	{
		//Health
	//	infile.getline(characters, 256,' ');
	//	infile.ignore(256, '\n');
	//	numberFromFile = atoi(characters);
	//	this->SetHealth(numberFromFile);

		////Max Alertness
		//infile.getline(characters, 256,' ');
		//infile.ignore(256, '\n');
		//numberFromFile = atoi(characters);
		//this->SetAlertnessMax(numberFromFile);

		////Initial Alertness
		//infile.getline(characters, 256,' ');
		//infile.ignore(256, '\n');
		//numberFromFile = atoi(characters);
		//this->SetAlertnessLevel(numberFromFile);

		//Max fear
		infile.getline(characters, 256,' ');
		infile.ignore(256, '\n');
		numberFromFile = atoi(characters);
		this->SetFearMax(numberFromFile);

	}
	else
	{
		throw("File could not be opened!");
	}
	if(infile.is_open())
	{
		infile.close();
	}
	this->SetFearLevel(0);
	this->zFearIntervalCounter = 0;
	//this->SetAlertnessLevel(0);
	//this->zAlertnessIntervalCounter = 0;
	this->SetBehaviour(CALM);
	this->zCalmActionInterval = rand () % 5 + 3; 
	this->SetIfNeedPath(true);

	this->SetPreviousHealth( this->GetHealth() );

}

void WolfActor::Update( float deltaTime ) //Has become a rather large funtion. Could probably use some optimization and downsizing.
{
	if(this->GetIfPlayerControlled() == true)
	{
		//this->UpdateForPlayer(deltaTime);
	}
	else
	{
		//this->UpdateForAnimal(deltaTime);
	}
	
}

void WolfActor::UpdateForAnimal(float deltaTime)
{
	this->zIntervalCounter += deltaTime;
	this->zFearIntervalCounter += deltaTime;
	//this->zAlertnessIntervalCounter += deltaTime;
	
	int nrOfPredators = 0;
	bool nearbyPredatorsExist = false;

		//Problems:
		//- How does it know about loud sounds? Some sort of event? 
		//- How does it know when nearby sneaking entities make a sound?
				//- Possible solution: getting the player's velocity could be enough.

	//Perform checking for entities here.

	int minimumDistance = 60; //60 is just a test value, I do not know the proper scale.
	int shortestDistance = 99999;

	float xDistance = 0;
	float yDistance = 0;
	float zDistance = 0;
	float finalDistance = 0;

	

	for(int i = 0; i < this->GetCurrentPlayers(); i++)
	{
		xDistance = this->GetPosition().x - this->zTargets[i].position.x; //Math, could use optimization, I think.
		//yDistance = this->GetPosition().y - this->zTargets[i].position.y;
		zDistance = this->GetPosition().z - this->zTargets[i].position.z;
		finalDistance = sqrt(xDistance * xDistance + zDistance * zDistance);
		if( finalDistance < minimumDistance ) 
		{
			this->zTargets[i].valid = true;
			if(finalDistance < shortestDistance)
			{
				shortestDistance = finalDistance;
				this->zMainTarget = this->zTargets[i]; //Decide which is the biggest threat here, i.e. the main target. For the moment, proximity is the deciding factor. Could use some more complexity.
			}
			nrOfPredators++;
		}
		else
		{
			this->zTargets[i].valid = false;
		}
		
	}
	if(nrOfPredators > 0)
	{
		nearbyPredatorsExist = true;
	}
	else
	{
		nearbyPredatorsExist = false;
	}



	/*if(this->zAlertnessIntervalCounter > 1)
	{
		this->SetAlertnessLevel( this->GetFearLevel() - 1);
	}*/

	
	//Time to assess threats.

	if( this->GetHealth() < this->GetPreviousHealth() ) //In theory, used to check if the animal has been attacked.
	{
		this->SetFearLevel( this->GetFearLevel() + 8);
	}
	this->SetPreviousHealth( this->GetHealth() );
	
	if(this->zFearIntervalCounter > 1)
	{
		this->zFearIntervalCounter = 0;
		//Basically, each second the fear increases or decreases.
		if(nearbyPredatorsExist)
		{
			//Getting values and such, comparing animal health against that of other entities, types and more.

			//calculate current fear level:
			float fear = 0;
			//- for each dangerous entity detected.
			//fear += 5 * nrOfPredators;	//As wolves are brave, and are more likely to attack, 
			
			if(shortestDistance < minimumDistance / 3) //The target is too close. Could expand this to incorporate more than one target.
			{
				fear += 5;
			}

			for(int i = 0; i < this->GetCurrentPlayers(); i++)
			{
				if(this->zTargets[i].valid == true)
				{
				//Do a mathematical check, see if anyone is right in front of the wolf. But... how? http://www.youtube.com/watch?v=gENVB6tjq_M
					float dotProduct = this->GetDirection().GetDotProduct( this->zTargets[i].position - this->GetPosition() );
					if(dotProduct > 0.7)//This sight is relatively narrrow, since it is a wolf. If this is true, then the wolf sees a player.
					{
						//Which means, it is even more afraid.
						fear += 5;
					}
					if(this->zTargets[i].movementNoise > 5) //5 is just a temporary number right now. It is supposed to be the speed of a running player.
					{
						fear += 2;
					}

					if(this->zTargets[i].health != 0) // No dbz here!
					{
						fear += (this->GetHealth() / this->zTargets[i].health) / nrOfPredators; //If the animal is faced with a very weak player(s), it gets some confidence. This is reduced with each player present.
					}
				}
			}			

			this->SetFearLevel( this->GetFearLevel() + fear / 5); //Possibly not good enough math. The theory is that the animal is constantly getting more afraid.
	
		}
		else //No threat detected. Calming down.
		{
			this->SetFearLevel( this->GetFearLevel() - 10);
		}
	}
		
	
	
	
	
	if(this->GetFearLevel() == 0 && !nearbyPredatorsExist)
	{
		this->SetBehaviour(CALM);
	}
	else if(this->GetFearLevel() > 0 && this->GetFearLevel() <= 10 /*|| this->GetAlertnessLevel() > 20*/)
	{
		this->SetBehaviour(SUSPICIOUS);
	}
	else if(this->zHealth < 10)
	{
		this->SetBehaviour(AFRAID);
	}
	else if(this->GetFearLevel() > 10 && this->GetFearLevel() <= this->GetFearMax() && nearbyPredatorsExist)
	{
		this->SetBehaviour(AGGRESSIVE);
	}
	else
	{
		this->SetBehaviour(SUSPICIOUS);
	}
	
	
	//	
	if(this->GetBehaviour() == CALM) //Relaxed behaviour. No threat detected.
	{
		if(this->zIntervalCounter > this->zCalmActionInterval)
		{
			this->zIntervalCounter = 0;
			this->zCalmActionInterval = rand () % 5 + 3; 
			this->zCurrentPath.clear(); //Since a new path is gotten, and the old one might not have been completed, we clear it just in case.
			this->zPathfinder.Pathfinding(this->GetPosition().x, this->GetPosition().z, this->GetPosition().x + rand() % 50, this->GetPosition().z + rand() % 50, this->zCurrentPath, 50); //Get a small path to walk, short and does not have to lead anywhere.
			this->SetIfNeedPath(true);
		}
	}
	else if(this->GetBehaviour() == SUSPICIOUS) //Might have heard something, is suspicious.
	{
		if(this->zIntervalCounter > this->zCalmActionInterval * 1.5) //The increase in time is supposed to represent listening, waiting for something to happen.
		{
			this->zIntervalCounter = 0;
			this->zCalmActionInterval = rand () % 5 + 3;
			this->zCurrentPath.clear();
			this->SetIfNeedPath(true);
			this->zPathfinder.Pathfinding(this->GetPosition().x, this->GetPosition().z, this->GetPosition().x + rand() % 10, this->GetPosition().z + rand() % 10, this->zCurrentPath, 20); //Get a small path to walk, quite short (since the animal is nervous) and does not have to lead anywhere.

		}
	}
	else if(this->GetBehaviour() == AGGRESSIVE) //Is outright trying to harm the target.
	{
		xDistance = this->GetPosition().x - this->zMainTarget.position.x;
		//yDistance = this->GetPosition().y - this->zMainTarget.position.y;
		zDistance = this->GetPosition().z - this->zMainTarget.position.z;
		float lastDistance = sqrt(xDistance * xDistance + zDistance * zDistance);
		if( this->GetIfNeedPath() == true )
		{
			this->SetIfNeedPath(false);
			this->zCurrentPath.clear();
			if( !this->zPathfinder.Pathfinding(this->GetPosition().x, this->GetPosition().z, this->zMainTarget.position.x, this->zMainTarget.position.z, this->zCurrentPath, 100) == false ); //Get the path, with the target that is to be attacked as the goal position. Depending on the animal, make the distance slightly large.
			{
				this->SetIfNeedPath(true);
			}
			this->SetLastDistanceCheck( lastDistance );
		}
		if( this->zIntervalCounter > 1 && this->GetIfNeedPath() == false )
		{
			this->zIntervalCounter = 0;

			if( lastDistance < this->GetLastDistanceCheck() / 2) // The animal has traveled towards its goal halfway, at this point, it is safe to asume the goal has moved.
			{
				this->zCurrentPath.clear();
				this->zPathfinder.Pathfinding(this->GetPosition().x, this->GetPosition().z, this->zMainTarget.position.x, this->zMainTarget.position.z, this->zCurrentPath, 100);
			}

			xDistance = 0;
			yDistance = 0;
			zDistance = 0;
			float distance;
			float shortestDistance = 99999;
			Target mostLikelyTarget = this->zMainTarget;

			for(int i = 0; i < this->GetCurrentPlayers(); i++)
			{
				
				if(this->zTargets[i].valid == true)
				{
					xDistance = this->GetPosition().x - this->zTargets[i].position.x;
					//yDistance = this->GetPosition().y - this->zTargets[i].position.y;
					zDistance = this->GetPosition().z - this->zTargets[i].position.z;
					distance = sqrt(xDistance * xDistance + zDistance * zDistance);
					
					if(distance < shortestDistance) //Something that is a larger threat is based on distance.
					{
						shortestDistance = distance;
						mostLikelyTarget = this->zTargets[i];
					}
				}
			}
			if(shortestDistance < this->GetLastDistanceCheck() / 3) // The animal has gotten closer to a larger threat, and is now following that target instead.
			{
				this->SetIfNeedPath(true);
				this->zMainTarget = mostLikelyTarget;
			}
		}
		
	}
	else if(this->GetBehaviour() == AFRAID) //Is afraid, running.
	{
		if(this->GetIfNeedPath() == true)
		{
			this->SetIfNeedPath(false);
			float awayFromThreatX;
			float awayFromThreatZ;

			if(nearbyPredatorsExist)
			{
				if(this->zMainTarget.position.x < this->GetPosition().x )
				{
					awayFromThreatX = this->GetPosition().x + 80 + (rand() % 50 - 25 );//I just want to add a bit of a random element
				}
				else
				{
					awayFromThreatX = this->GetPosition().x - 80 + (rand() % 50 - 25 );
				}
				if(this->zMainTarget.position.z < this->GetPosition().z )
				{
					awayFromThreatZ = this->GetPosition().z + 80 + (rand() % 50 - 25 );
				}
				else
				{
					awayFromThreatZ = this->GetPosition().z - 80 + (rand() % 50 - 25 );
				}

			}
			else
			{
				//Get random direction and run there.
				int directionX = rand() % 1; 
				int directionZ = rand() % 1; 
				if(directionX == 0)
				{
					awayFromThreatX = this->GetPosition().x + 80;
				}
				else
				{
					awayFromThreatX = this->GetPosition().x - 80;
				}
				if(directionZ == 0)
				{
					awayFromThreatZ = this->GetPosition().z + 80;
				}
				else
				{
					awayFromThreatZ = this->GetPosition().z - 80;
				}
				
			}
			this->zCurrentPath.clear();
			if( !this->zPathfinder.Pathfinding(this->GetPosition().x, this->GetPosition().z, awayFromThreatX, awayFromThreatZ,this->zCurrentPath,100) )
			{
				this->SetIfNeedPath(true);
			}
		}
	}
	
}

void WolfActor::UpdateForPlayer(float deltaTime)
{

}

void WolfActor::LargeSuddenSoundReaction()
{
	this->SetFearLevel( this->GetFearLevel() + 30);
}