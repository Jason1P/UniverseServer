#pragma once
#include <vector>
#include "Database.h"
#include "NPCObject.h"
#include "EnvironmentalObject.h"
#include "EnemyObject.h"

struct WorldObjectInfo {
	std::wstring name = L"";
	unsigned long LOT = 0;
	short zone = 0;
	float posX;
	float posY;
	float posZ;
	float rotX;
	float rotY;
	float rotZ;
	float rotW;
};

class WorldObjectsTable {
public:
	static unsigned long long registerObject(std::wstring name, long long firstObjID, unsigned long LOT, unsigned short zone, float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float rotW);
	static void unregisterObject(unsigned long long id);
	static std::vector<unsigned long long> getObjects(unsigned short zone);
	static std::vector<unsigned long long> getObjects();
	static unsigned long getLOT(unsigned long long);
	static long long getTempObjID(unsigned long long id);
	static unsigned long long getID(long long tempObjID);
	static WorldObjectInfo getObjectInfo(unsigned long long id);
	static void updateTempObjID(unsigned long long id, long long tempObjID);
};