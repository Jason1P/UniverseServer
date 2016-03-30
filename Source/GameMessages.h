#pragma once
#include "World.h"
#include "Common.h"
#include "ReplicaComponents.h"

#include <string>

enum ObjectType{
	NONE = -1,
	LaunchPad,
	PostBox,
	Binoculars,
	Minigame,
	Plaque,
	Portal,
	Vault,
	WaterPot,
	Machine,
	Object
};

struct ObjectInformation{
	long long objid;
	ZoneId zone;
	ObjectType type;
	std::string info;
	ObjectInformation(long long id, ZoneId z, ObjectType t, std::string s = ""){
		this->objid = id;
		this->zone = z;
		this->type = t;
		this->info = s;
	}
	ObjectInformation(){
		this->objid = 0;
		this->zone = ZoneId::NO_ZONE;
		this->type = ObjectType::NONE;
		this->info = "";
	}
};

ObjectInformation getObjectInformation(long long objectid);
std::string getObjectDescription(ObjectInformation obj, long long objid = -1);
ZoneId getLaunchPadTarget(ObjectInformation obj);

class GameMSG {
public:
	static void addSkill(long long charid, unsigned long skillid, unsigned long slot);
	static void removeSkill(long long charid, unsigned long skillid);
	static void castActiveSkill(long long charid, unsigned long someNumber);
	static void addItemToInventoryClientSync(long long charid, long long objectID, long lotTemplate, long slotid, bool showFlyingLoot);
	//static void setLoseCoinsOnDeath(long long charid, bool loseCoinsOnDeath);
	static void playAnimation(long long charid, std::wstring animationID, bool playImmediate = true);
	static void setName(long long charid, std::wstring name);
	static void displayMessageBox(long long charid, long long callbackClient, std::wstring identifier, std::wstring text, std::wstring userData);
	static void displayZoneSummary(long long charid, bool isProperty, bool isZoneStart);
	static void teleport(long long charid, COMPONENT1_POSITION position);
	static void smash(long long objectID, bool ignoreObjectVisibility, float force, float ghostOpacity, long long killerID);
	static void setJetPackMode(long long charid, bool bypassChecks, bool doHover, bool use);
	static void dropClientLoot(long long charid, float posX, float posY, float posZ, long currency, unsigned long LOT, long long objid, long long ownerid, long long sourceid);
	static void knockback(long long charid, long long casterid, float vecX, float vecY, float vecZ);
};