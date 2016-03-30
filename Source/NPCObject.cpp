#include "Database.h"
#include "NPCObject.h"
#include "Replica.h"
#include "World.h"
#include "WorldServer.h"
#include "Worlds.h"
#include "CDClientDB.h"
#include "PlayerObject.h"
#include "AccountsDB.h"
#include "InventoryDB.h"

#include "ObjectID.h"

#include "RakNet\ReplicaManager.h"

#include "Logger.h"
#include "UtfConverter.h"

// SQLite
#include "SQLiteDatabase.h"

#include <map>

using namespace std;

NPCObject::NPCObject(unsigned long lot, unsigned long currentZone) {
	Logger::log("REPL", "NPC", "Initializing NPC Object");

	this->zoneID = currentZone;
	initializeObject(lot);
}

NPCObject::~NPCObject() {

}

void NPCObject::initializeObject(unsigned long lot) {
	this->objid = ObjectID::generateObjectID();
	this->LOT = lot;

	SpawnerObject *spawner = new SpawnerObject(ObjectID::generateSpawnerID());
	this->spawnerObject = spawner;
	this->spawnerObjID = spawner->objid;
	this->spawner_node_id = 0;

	this->addComponent(new SimplePhysicsComponent());
	this->addComponent(new DestructibleComponent());

	this->world.zone = this->zoneID;

	COMPONENT7_DATA3 d3 = COMPONENT7_DATA3();
	d3.d1 = 0; d3.d2 = 0; d3.d3 = 0; d3.d4 = 0; d3.d5 = 0; d3.d6 = 0; d3.d7 = 0; d3.d8 = 0; d3.d9 = 0;

	DestructibleComponent *c7 = this->getDestructibleComponent();
	c7->setData3(d3);

	std::vector<long> destructibleData = CDClientDB::getDestructibleComponentData(lot);
	if (destructibleData.size() > 0) {
		COMPONENT7_DATA4 d4 = COMPONENT7_DATA4();
		d4.health = destructibleData.at(1);
		d4.maxHealth = destructibleData.at(1);
		d4.maxHealthN = destructibleData.at(1);

		d4.imagination = destructibleData.at(2);
		d4.maxImagination = destructibleData.at(2);
		d4.maxImaginationN = destructibleData.at(2);

		d4.armor = destructibleData.at(3);
		d4.maxArmor = destructibleData.at(3);
		d4.maxArmorN = destructibleData.at(3);

		c7->setData4(d4);
		c7->getData4_1Ref()->push_back(destructibleData.at(0));
		c7->setData4_4_1(true);
		c7->setData5(false);
	}

	std::vector<unsigned long> equipment = CDClientDB::getInventoryComponentData(lot);
	if (equipment.size() > 0) {
		this->isInventoryAdded = true;
		this->addComponent(new InventoryComponent());

		InventoryComponent *c17 = this->getInventoryComponent();

		for (int i = 0; i < equipment.size(); i++) {
			LootObject *lootObj = new LootObject(equipment.at(i), ObjectID::generateObjectID());

			c17->equipNPCItem(equipment.at(i), lootObj->objid);
			equippedItems.push_back(lootObj);
		}
	}

	unsigned long componentID = CDClientDB::getComponentID(lot, 5);
	if (componentID != -1) {
		this->addComponent(new ScriptComponent());
		this->isScriptAdded = true;
	}

	componentID = CDClientDB::getComponentID(lot, 9);
	if (componentID != -1) {
		this->addComponent(new SkillComponent());
		this->isSkillAdded = true;
	}

	componentID = CDClientDB::getComponentID(lot, 16);
	if (componentID != -1) {
		this->addComponent(new VendorComponent());
		this->isVendor = true;
	}

	this->addComponent(new RenderComponent());
}

void NPCObject::deleteSubObjects() {
	if (this->isInventoryAdded) {
		for (int k = 0; k < this->equippedItems.size(); k++) {
			ObjectsManager::unregisterObject(this->equippedItems.at(k));
		}
	}
	ObjectsManager::unregisterObject(this->spawnerObject);
}

long long NPCObject::getObjectID() { return this->objid; }
long long NPCObject::getSpawnerObjectID() { return this->spawnerObjID; }
unsigned long NPCObject::getLOT() { return this->LOT;  }

void NPCObject::setPosition(float posX, float posY, float posZ) {
	SimplePhysicsComponent *c3 = this->getSimplePhysicsComponent();

	COMPONENT3_POSITION pos = c3->getPosition();
	pos.posX = posX;
	pos.posY = posY;
	pos.posZ = posZ;
	c3->setPosition(pos);
}

void NPCObject::setRotation(float rotX, float rotY, float rotZ, float rotW) {
	SimplePhysicsComponent *c3 = this->getSimplePhysicsComponent();

	COMPONENT3_ROTATION rot = c3->getRotation();
	rot.rotX = rotX;
	rot.rotY = rotY;
	rot.rotZ = rotZ;
	rot.rotW = rotW;
	c3->setRotation(rot);
}

SimplePhysicsComponent *NPCObject::getSimplePhysicsComponent() { return (SimplePhysicsComponent *)this->getComponent(3); }
DestructibleComponent *NPCObject::getDestructibleComponent() { return (DestructibleComponent *)this->getComponent(7); }
InventoryComponent *NPCObject::getInventoryComponent() { if (isInventoryAdded) { return (InventoryComponent *)this->getComponent(17); } else { return NULL; } }
ScriptComponent *NPCObject::getScriptComponent() { if (isScriptAdded){ return (ScriptComponent *)this->getComponent(5); } else { return NULL; } }
SkillComponent *NPCObject::getSkillComponent() { if (isSkillAdded){ return (SkillComponent *)this->getComponent(9); } else { return NULL; } }
VendorComponent *NPCObject::getVendorComponent() { if (isVendor){ return (VendorComponent *)this->getComponent(16); } else { return NULL; } }
RenderComponent *NPCObject::getRenderComponent() { return (RenderComponent *)this->getComponent(2); }

bool NPCObject::didAddInventory() { return this->isInventoryAdded; }
bool NPCObject::didAddVendorComponent() { return this->isVendor; }
bool NPCObject::didAddScript() { return this->isScriptAdded; }
bool NPCObject::didAddSkill() { return this->isSkillAdded; }