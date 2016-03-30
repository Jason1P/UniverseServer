#include "FileConfiguration.h"

FileConfiguration::FileConfiguration(std::string filename) {
	_filename = filename;

	std::ifstream file(filename, std::ios::binary);
	bool succeeded = false;
	while (!succeeded) {
		if (doesFileExist(filename)) {
			if (file.is_open()) {
				std::string line = "";
				while (std::getline(file, line)) {
					std::vector<std::string> section = split(line, '=');
					_loadedData[section.at(0)] = section.at(1);
				}
				file.close();

				succeeded = true;
			}
		}
		else
			succeeded = true;
	}
}

FileConfiguration FileConfiguration::loadConfiguration(std::string filename) {
	return FileConfiguration(filename);
}

void FileConfiguration::writeByte(std::string section, char b) {
	_loadedData[section] = std::to_string((int)b);
}

void FileConfiguration::writeInteger(std::string section, int i) {
	_loadedData[section] = std::to_string(i);
}

void FileConfiguration::writeBoolean(std::string section, bool b) {
	if (b)
		_loadedData[section] = "true";
	else
		_loadedData[section] = "false";
}

void FileConfiguration::writeString(std::string section, std::string str) {
	_loadedData[section] = str;
}

char FileConfiguration::readByte(std::string section, char defaultV) {
	if (_loadedData.size() > 0) {
		if (_loadedData.find(section) != _loadedData.end()) {
			int i = readInteger(section, 127);
			if (i < 127)
				return i;
			else
				return defaultV;
		}
		else
			return defaultV;
	}
	else
		return defaultV;
}

int FileConfiguration::readInteger(std::string section, int defaultV) {
	if (_loadedData.size() > 0) {
		if (_loadedData.find(section) != _loadedData.end())
			return std::stoi(_loadedData[section]);
		else
			return defaultV;
	}
	else
		return defaultV;
}

bool FileConfiguration::readBoolean(std::string section, bool defaultV) {
	if (_loadedData.size() > 0) {
		if (_loadedData.find(section) != _loadedData.end()) {
			if (_loadedData[section] == "false")
				return false;
			else if (_loadedData[section] == "true")
				return true;
			else
				return defaultV;
		}
		else
			return defaultV;
	}
	else
		return defaultV;
}

std::string FileConfiguration::readString(std::string section, std::string defaultV) {
	if (_loadedData.size() > 0) {
		if (_loadedData.find(section) != _loadedData.end())
			return _loadedData[section];
		else
			return defaultV;
	}
	else
		return defaultV;
}

void FileConfiguration::save() {
	std::ofstream file(_filename, std::ios::binary);
	bool succeeded = false;
	while (!succeeded) {
		if (doesFileExist(_filename)) {
			if (file.is_open()) {
				std::map<std::string, std::string>::iterator it = _loadedData.begin();
				while (it != _loadedData.end()) {
					file << it->first << "=" << it->second << "\n";
					it++;
				}
				file.close();

				succeeded = true;
			}
		}
		else {
			std::map<std::string, std::string>::iterator it = _loadedData.begin();
			while (it != _loadedData.end()) {
				file << it->first << "=" << it->second << "\n";
				it++;
			}
			file.close();

			succeeded = true;
		}
	}
}