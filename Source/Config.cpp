#include "Config.h"
#include "../../FileConfiguration.h"

bool Config::getLogFile() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readBoolean("General.logFile", true);
}

bool Config::getUseEncryption() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readBoolean("General.useEncryption", false);
}

std::string Config::getIP(std::string role) {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readString("Servers." + role + ".address", "127.0.0.1");
}

int Config::getPort(std::string role) {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readInteger("Servers." + role + ".port", 0);
}

std::string Config::getMySQLHost() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readString("MySQL.host", "127.0.0.1");
}

std::string Config::getMySQLDatabase() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readString("MySQL.database", "luni");
}

std::string Config::getMySQLUsername() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readString("MySQL.username", "root");
}

std::string Config::getMySQLPassword() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
	return fs.readString("MySQL.password", "");
}

//void Config::initConnectInfo(CONNECT_INFO& info, std::string setting) {
//	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");
//
//	strcpy(info.redirectIp, fs.readString("redirectIP", "127.0.0.1").c_str());
//
//	info.logFile = fs.readBoolean("logFile", true);
//	info.useEncryption = fs.readBoolean("useEncryption", false);
//
//	info.listenPort = fs.readInteger(setting + ".listenPort", 0);
//	info.redirectPort = fs.readInteger(setting + ".redirectPort", 0);
//}

void Config::generateDefaultConfig() {
	FileConfiguration fs = FileConfiguration::loadConfiguration(".\\config.cfg");

	fs.writeBoolean("General.logFile", true);
	fs.writeBoolean("General.useEncryption", false);

	fs.writeString("MySQL.host", "127.0.0.1");
	fs.writeString("MySQL.database", "luni");
	fs.writeString("MySQL.username", "root");
	fs.writeString("MySQL.password", "");

	fs.writeString("Servers.Auth.address", "127.0.0.1");
	fs.writeInteger("Servers.Auth.port", 1001);
	fs.writeString("Servers.World.address", "127.0.0.1");
	fs.writeInteger("Servers.World.port", 2002);
	fs.writeString("Servers.Chat.address", "127.0.0.1");
	fs.writeInteger("Servers.Chat.port", 2003);

	fs.save();
}