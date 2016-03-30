#pragma once
#include "RakNet\RakNetTypes.h"
#include "Database.h"

enum ServerRole : unsigned char{
	ROLE_CONSOLE = 0,
	ROLE_AUTH,
	ROLE_WORLD,
	ROLE_CHAT
};

class InstancesTable {
public:
	//Registers a server instance on this ip&port, returns the instanceid
	static int registerInstance(SystemAddress addr);
	static int getInstanceId(SystemAddress addr);
	static void unregisterInstance(SystemAddress addr);
};