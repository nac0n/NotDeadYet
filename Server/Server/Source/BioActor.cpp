#include "BioActor.h"


BioActor::BioActor() : Actor()
{
	this->zPlayer = NULL;

	zInventory = new Inventory();
	this->zState = STATE_IDLE;
	this->zVelocity = V_WALK_SPEED;

	this->zAlive = true;
	this->zHealthMax = 100;
	this->zHealth = zHealthMax;

	this->zStaminaMax = 100;
	this->zStamina = zStaminaMax;
	this->zStaminaCof = 2.2f;

	this->zBleedingLevel = 0.0f;
	this->zValid = false;
}

BioActor::BioActor(Player* player) : Actor()
{
	this->zPlayer = player;

	zInventory = new Inventory();
	this->zState = STATE_IDLE;
	this->zVelocity = V_WALK_SPEED;

	this->zAlive = true;
	this->zHealthMax = 100;
	this->zHealth = zHealthMax;

	this->zStaminaMax = 100;
	this->zStamina = zStaminaMax;
	this->zStaminaCof = 2.2f;

	this->zBleedingLevel = 0.0f;
	this->zValid = false;
}

BioActor::~BioActor()
{
	SAFE_DELETE(this->zInventory);
}

bool BioActor::TakeDamage(Damage& dmg, Actor* dealer)
{
	// Notify Damage
	BioActorTakeDamageEvent BATD;
	BATD.zActor = this;
	BATD.zDamage = &dmg;
	BATD.zDealer = dealer;
	NotifyObservers(&BATD);

	this->zHealth -= dmg.GetTotal();

	if(this->zHealth <= 0.0f)
	{
		this->zAlive = false;
		this->zHealth = 0.0f;
	}

	if(dmg.GetTotal() / this->zHealth > 0.20 && dmg.GetBleedFactor() > 0.6)
	{
		float currentBleeding = this->zBleedingLevel;
		this->SetBleeding( currentBleeding + 1 );
	}

	if(!zAlive)
	{
		//RotateMesh
		Vector3 up = Vector3(0, 1, 0);
		Vector3 forward = this->GetDir();
		forward.y = 0;
		forward.Normalize();
		Vector3 around = up.GetCrossProduct(forward);
		around.Normalize();
		float angle = 3.14f * 0.5f;
		
		this->GetPhysicsObject()->SetQuaternion(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		this->SetRotation(around,angle);
	}
	
	return this->zAlive;
}

void BioActor::Kill()
{
	Damage dmg;
	dmg.fallingDamage = zHealth;

	TakeDamage(dmg, this);
}

bool BioActor::IsAlive() const
{
	return this->zAlive;
}

bool BioActor::HasMoved()
{
	Vector3 curPos = GetPosition();

	if(curPos == GetPreviousPos())
		return false;	

	return true;
}

void BioActor::SetState( const int state, const bool notify /*= true*/ )
{
	if (state != this->zState)
	{
		this->zState = state;

		if(notify)
		{
			BioActorStateEvent BASE;
			BASE.zBioActor = this;
			NotifyObservers(&BASE);
		}
	}
}

void BioActor::SetBleeding( const float levelBleeding, const bool notify /*= true*/ )
{
	if( levelBleeding != this->zBleedingLevel )
	{
		this->zBleedingLevel = levelBleeding;

		if(this->zBleedingLevel < 0)
			this->zBleedingLevel = 0;

		if(notify)
		{
			BioActorPhysicalConditionBleedingEvent e;
			e.zBioActor = this;
			NotifyObservers(&e);
		}
	}
}

void BioActor::SetHealth( const float health, const bool notify /*= true*/)
{
	if(this->zHealth != health)
	{
		this->zHealth = health;

		if(notify)
		{
			BioActorPhysicalConditionHealthEvent e;
			e.zBioActor = this;
			NotifyObservers(&e);
		}
	}
}

void BioActor::SetStamina( const float stamina, const bool notify /*= true*/ )
{
	if(this->zStamina != stamina )
	{
		this->zStamina = stamina;

		if(notify)
		{
			BioActorPhysicalConditionStaminaEvent e;
			e.zBioActor = this;
			NotifyObservers(&e);
		}
	}
}