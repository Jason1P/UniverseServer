#pragma once

#include "ReplicaData.h"
#include "ReplicaComponents.h"
#include "ReplicaObject.h"
#include "SpawnerObject.h"
#include "LootObject.h"

#include "RakNet\Replica.h"

#include <string>

class EnemyObject : public ReplicaObject {
private:
	SystemAddress clientIP;
	SpawnerObject *spawnerObject = NULL;

	unsigned long zoneID = 0;

	bool isInventoryAdded = false;

	void initializeObject(unsigned long lot);

	std::vector<LootObject *> equippedItems;
public:
	EnemyObject(unsigned long lot, unsigned long currentZone); // Will call initializeObject()
	~EnemyObject();

	void deleteSubObjects();
	long long getObjectID();
	long long getSpawnerObjectID();
	unsigned long getLOT();

	void setPosition(float posX, float posY, float posZ);
	void setRotation(float rotX, float rotY, float rotZ, float rotW);

	ControllablePhysicsComponent *getControllablePhysicsComponent();
	DestructibleComponent *getDestructibleComponent();
	InventoryComponent *getInventoryComponent();
	ScriptComponent *getScriptComponent();
	SkillComponent *getSkillComponent();
	BaseCombatAIComponent *getBaseCombatAIComponent();
	RenderComponent *getRenderComponent();
};