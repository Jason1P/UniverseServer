#include "EnvironmentalObject.h"
#include "Worlds.h"
#include "PlayerObject.h"

#include "ObjectID.h"

#include "RakNet\ReplicaManager.h"

#include "Logger.h"
#include "UtfConverter.h"
#include "CDClientDB.h"

#include "SQLiteDatabase.h"

#include <map>
#include <sstream>

using namespace std;

EnvironmentalObject::EnvironmentalObject(unsigned long lot, unsigned long currentZone) {
	Logger::log("REPL", "ENV", "Initializing Environmental Object");

	this->zoneID = currentZone;
	initializeObject(lot);
}

EnvironmentalObject::~EnvironmentalObject() {

}

void EnvironmentalObject::initializeObject(unsigned long lot) {
	this->objid = ObjectID::generateObjectID();
	this->LOT = lot;

	SpawnerObject *spawner = new SpawnerObject(ObjectID::generateSpawnerID());
	this->spawnerObject = spawner;
	this->spawnerObjID = spawner->objid;
	this->spawner_node_id = 0;

	unsigned long componentID = CDClientDB::getComponentID(lot, 3);
	if (componentID != -1) {
		this->isSimplyPhysics = true;
		this->addComponent(new SimplePhysicsComponent());
	}
	else {
		this->isSimplyPhysics = false;
		this->addComponent(new PhantomPhysicsComponent());
	}

	this->world.zone = this->zoneID;

	std::vector<long> destructibleData = CDClientDB::getDestructibleComponentData(lot);
	if (destructibleData.size() > 0) {
		this->isDestructible = true;
		this->addComponent(new DestructibleComponent());

		COMPONENT7_DATA3 d3 = COMPONENT7_DATA3();
		d3.d1 = 0; d3.d2 = 0; d3.d3 = 0; d3.d4 = 0; d3.d5 = 0; d3.d6 = 0; d3.d7 = 0; d3.d8 = 0; d3.d9 = 0;

		DestructibleComponent *c7 = this->getDestructibleComponent();
		c7->setData3(d3);

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

	componentID = CDClientDB::getComponentID(lot, 5);
	if (componentID != -1) {
		this->isScriptAdded = true;
		this->addComponent(new ScriptComponent());
	}

	componentID = CDClientDB::getComponentID(lot, 39);
	if (componentID != -1) {
		this->isScriptedActivityAdded = true;
		this->addComponent(new ScriptedActivityComponent());
	}

	this->addComponent(new RenderComponent());
}

long long EnvironmentalObject::getObjectID() { return this->objid; }
long long EnvironmentalObject::getSpawnerObjectID() { return this->spawnerObjID; }
unsigned long EnvironmentalObject::getLOT() { return this->LOT; }

void EnvironmentalObject::setPosition(float posX, float posY, float posZ) {
	if (this->isSimplyPhysics) {
		SimplePhysicsComponent *c3 = this->getSimplePhysicsComponent();

		COMPONENT3_POSITION pos = c3->getPosition();
		pos.posX = posX;
		pos.posY = posY;
		pos.posZ = posZ;
		c3->setPosition(pos);
	}
	else {
		PhantomPhysicsComponent *c40 = this->getPhantomPhysicsComponent();

		COMPONENT40_POSITION pos = c40->getPosition();
		pos.posX = posX;
		pos.posY = posY;
		pos.posZ = posZ;
		c40->setPosition(pos);
	}
}

void EnvironmentalObject::setRotation(float rotX, float rotY, float rotZ, float rotW) {
	if (this->isSimplyPhysics) {
		SimplePhysicsComponent *c3 = this->getSimplePhysicsComponent();

		COMPONENT3_ROTATION rot = c3->getRotation();
		rot.rotX = rotX;
		rot.rotY = rotY;
		rot.rotZ = rotZ;
		rot.rotW = rotW;
		c3->setRotation(rot);
	}
	else {
		PhantomPhysicsComponent *c40 = this->getPhantomPhysicsComponent();

		COMPONENT40_ROTATION rot = c40->getRotation();
		rot.rotX = rotX;
		rot.rotY = rotY;
		rot.rotZ = rotZ;
		rot.rotW = rotW;
		c40->setRotation(rot);
	}
}

SimplePhysicsComponent *EnvironmentalObject::getSimplePhysicsComponent(){ if (this->isSimplyPhysics) { return (SimplePhysicsComponent*)this->getComponent(3); } else { return NULL; } }
PhantomPhysicsComponent *EnvironmentalObject::getPhantomPhysicsComponent(){ if (!this->isSimplyPhysics) { return (PhantomPhysicsComponent*)this->getComponent(40); } else { return NULL; } }
DestructibleComponent *EnvironmentalObject::getDestructibleComponent(){ if (this->isDestructible) { return (DestructibleComponent*)this->getComponent(7); } else { return NULL; } }
ScriptComponent *EnvironmentalObject::getScriptComponent(){ if (this->isScriptAdded) { return (ScriptComponent*)this->getComponent(5); } else { return NULL; } }
ScriptedActivityComponent *EnvironmentalObject::getScriptedActivityComponent(){ if (this->isScriptedActivityAdded) { return (ScriptedActivityComponent*)this->getComponent(39); } else { return NULL; } }
RenderComponent *EnvironmentalObject::getRenderComponent(){ return (RenderComponent*)this->getComponent(2); }

bool EnvironmentalObject::didUseSimplePhysics() { return this->isSimplyPhysics; }
bool EnvironmentalObject::didAddDestructibleComponent() { return this->isDestructible; }
bool EnvironmentalObject::didAddScript() { return this->isScriptAdded; }
bool EnvironmentalObject::didAddScriptedActivity() { return this->isScriptedActivityAdded; }