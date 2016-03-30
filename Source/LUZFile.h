#include "Common.h"
#include "serverLoop.h"
#include "LVLFile.h"

struct Spawnpoint {
	float posX = 0.0;
	float posY = 0.0;
	float posZ = 0.0;
	float rotX = 0.0;
	float rotY = 0.0;
	float rotZ = 0.0;
	float rotW = 0.0;
};

class LUZFile {
private:
	unsigned long version;
	unsigned long worldID;

	Spawnpoint spawnpoint;

	std::vector<LVLFile> children;
public:
	LUZFile();
	LUZFile(std::string filename);
	LUZFile(std::vector<unsigned char> data);
	~LUZFile();

	unsigned long getVersion();
	unsigned long getWorldID();
	
	Spawnpoint getSpawnpoint();

	std::vector<LVLFile> getChildren();
};