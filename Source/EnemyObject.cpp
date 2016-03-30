#include "Database.h"
#include "EnemyObject.h"
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

EnemyObject::EnemyObject(unsigned long lot, unsigned long currentZone) {
	Logger::log("REPL", "ENM", "Initializing Enemy Object");

	this->zoneID = currentZone;
	initializeObject(lot);
}

EnemyObject::~EnemyObject() {

}

void EnemyObject::initializeObject(unsigned long lot) {
	this->objid = ObjectID::generateObjectID();
	this->LOT = lot;

	SpawnerObject *spawner = new SpawnerObject(ObjectID::generateSpawnerID());
	this->spawnerObject = spawner;
	this->spawnerObjID = spawner->objid;
	this->spawner_node_id = 0;

	this->addComponent(new ControllablePhysicsComponent());
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
	
	this->addComponent(new ScriptComponent());
	this->addComponent(new SkillComponent());
	this->addComponent(new BaseCombatAIComponent());
	this->addComponent(new RenderComponent());
}

void EnemyObject::deleteSubObjects() {
	if (this->isInventoryAdded) {
		for (int k = 0; k < this->equippedItems.size(); k++) {
			ObjectsManager::unregisterObject(this->equippedItems.at(k));
		}
	}
	ObjectsManager::unregisterObject(this->spawnerObject);
}

long long EnemyObject::getObjectID() { return this->objid; }
long long EnemyObject::getSpawnerObjectID() { return this->spawnerObjID; }
unsigned long EnemyObject::getLOT() { return this->LOT; }

void EnemyObject::setPosition(float posX, float posY, float posZ) {
	ControllablePhysicsComponent *c1 = this->getControllablePhysicsComponent();

	COMPONENT1_POSITION pos = c1->getPosition();
	pos.x = posX;
	pos.y = posY;
	pos.z = posZ;
	c1->setPosition(pos);
}

void EnemyObject::setRotation(float rotX, float rotY, float rotZ, float rotW) {
	ControllablePhysicsComponent *c1 = this->getControllablePhysicsComponent();

	COMPONENT1_ROTATION rot = c1->getRotation();
	rot.x = rotX;
	rot.y = rotY;
	rot.z = rotZ;
	rot.w = rotW;
	c1->setRotation(rot);
}

ControllablePhysicsComponent *EnemyObject::getControllablePhysicsComponent() { return (ControllablePhysicsComponent*)this->getComponent(1); }
DestructibleComponent *EnemyObject::getDestructibleComponent() { return (DestructibleComponent*)this->getComponent(7); }
InventoryComponent *EnemyObject::getInventoryComponent() { if (isInventoryAdded) { return (InventoryComponent *)this->getComponent(17); } else { return NULL; } }
ScriptComponent *EnemyObject::getScriptComponent() { return (ScriptComponent*)this->getComponent(5); }
SkillComponent *EnemyObject::getSkillComponent() { return (SkillComponent*)this->getComponent(9); }
BaseCombatAIComponent *EnemyObject::getBaseCombatAIComponent() { return (BaseCombatAIComponent*)this->getComponent(60); }
RenderComponent *EnemyObject::getRenderComponent() { return (RenderComponent*)this->getComponent(2); }