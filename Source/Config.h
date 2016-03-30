#pragma once
#include "../../Common.h"

class Config {
public:
	static bool getLogFile();
	static bool getUseEncryption();

	static std::string getIP(std::string role);
	static int getPort(std::string role);

	static std::string getMySQLHost();
	static std::string getMySQLDatabase();
	static std::string getMySQLUsername();
	static std::string getMySQLPassword();

	//static void initConnectInfo(CONNECT_INFO& info, std::string setting);

	static void generateDefaultConfig();
};