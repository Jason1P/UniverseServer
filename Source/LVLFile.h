#include "Common.h"
#include "serverLoop.h"
#include "WorldObjectsDB.h"

class LVLFile {
private:
	std::vector<WorldObjectInfo> objects;
public:
	LVLFile(std::string filename, std::string scenename, unsigned long version, unsigned long worldID);
	~LVLFile();

	std::vector<WorldObjectInfo> getObjects();
};