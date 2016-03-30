#include "Common.h"

class FileConfiguration {
private:
	std::string _filename;
	std::map<std::string, std::string> _loadedData;

	FileConfiguration(std::string filename);
public:
	static FileConfiguration loadConfiguration(std::string filename);

	void writeByte(std::string section, char b);
	void writeInteger(std::string section, int i);
	void writeBoolean(std::string section, bool b);
	void writeString(std::string section, std::string str);

	char readByte(std::string section, char defaultV);
	int readInteger(std::string section, int defaultV);
	bool readBoolean(std::string section, bool defaultV);
	std::string readString(std::string section, std::string defaultV);

	void save();
};