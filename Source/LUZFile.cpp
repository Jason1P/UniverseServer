#include "LUZFile.h"
#include "Logger.h"

LUZFile::LUZFile() {};

LUZFile::LUZFile(std::string filename) {
	std::vector<unsigned char> content = OpenPacket(filename);

	RakNet::BitStream file;
	for (int i = 0; i < content.size(); i++) {
		file.Write(content.at(i));
	}

	unsigned long u32;
	unsigned char u8;

	file.Read(version);
	Logger::log("DEBUG", "LUZFile", "Version: " + std::to_string(version));

	if (version >= 0x24)
		file.Read(u32);

	file.Read(worldID);
	Logger::log("DEBUG", "LUZFile", "WorldID: " + std::to_string(worldID));

	if (version >= 0x26) {
		file.Read(spawnpoint.posX);
		file.Read(spawnpoint.posY);
		file.Read(spawnpoint.posZ);

		file.Read(spawnpoint.rotX);
		file.Read(spawnpoint.rotY);
		file.Read(spawnpoint.rotZ);
		file.Read(spawnpoint.rotW);
	}

	unsigned long scene_amount;

	if (version < 0x25) {
		unsigned char buf;
		file.Read(buf);
		scene_amount = buf;
	}
	else {
		file.Read(scene_amount);
	}

	for (int i = 0; i < scene_amount; i++) {
		unsigned char len;

		file.Read(len);

		std::string file_name = "";
		for (int k = 0; k < len; k++) {
			char buf;
			file.Read(buf);
			file_name += buf;
		}

		file.Read(u32);
		file.Read(u32);

		file.Read(len);

		std::string scene_name = "";
		for (int k = 0; k < len; k++) {
			char buf;
			file.Read(buf);
			scene_name += buf;
		}

		file.Read(u8);
		file.Read(u8);
		file.Read(u8);

		LVLFile child = LVLFile(".\\Files\\" + file_name, scene_name, version, worldID);
		children.push_back(child);
	}
}

LUZFile::LUZFile(std::vector<unsigned char> data) {
	RakNet::BitStream file;
	for (int i = 0; i < data.size(); i++) {
		file.Write(data.at(i));
	}

	unsigned long u32;
	unsigned char u8;

	file.Read(version);
	Logger::log("DEBUG", "LUZFile", "Version: " + std::to_string(version));

	if (version >= 0x24)
		file.Read(u32);

	file.Read(worldID);
	Logger::log("DEBUG", "LUZFile", "WorldID: " + std::to_string(worldID));

	if (version >= 0x26) {
		file.Read(spawnpoint.posX);
		file.Read(spawnpoint.posY);
		file.Read(spawnpoint.posZ);

		file.Read(spawnpoint.rotX);
		file.Read(spawnpoint.rotY);
		file.Read(spawnpoint.rotZ);
		file.Read(spawnpoint.rotW);
	}

	unsigned long scene_amount;

	if (version < 0x25) {
		unsigned char buf;
		file.Read(buf);
		scene_amount = buf;
	}
	else {
		file.Read(scene_amount);
	}

	for (int i = 0; i < scene_amount; i++) {
		unsigned char len;

		file.Read(len);

		std::string file_name = "";
		for (int k = 0; k < len; k++) {
			char buf;
			file.Read(buf);
			file_name += buf;
		}

		file.Read(u32);
		file.Read(u32);

		file.Read(len);

		std::string scene_name = "";
		for (int k = 0; k < len; k++) {
			char buf;
			file.Read(buf);
			scene_name += buf;
		}

		file.Read(u8);
		file.Read(u8);
		file.Read(u8);

		LVLFile child = LVLFile(".\\Files\\" + file_name, scene_name, version, worldID);
		children.push_back(child);
	}
}

LUZFile::~LUZFile() {}

unsigned long LUZFile::getVersion() { return version; }
unsigned long LUZFile::getWorldID() { return worldID; }

Spawnpoint LUZFile::getSpawnpoint() { return spawnpoint; }

std::vector<LVLFile> LUZFile::getChildren() { return children; }