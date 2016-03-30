#include "WorldObjectsDB.h"
#include "Logger.h"
#include "UtfConverter.h"

unsigned long long WorldObjectsTable::registerObject(std::wstring name, long long firstObjID, unsigned long LOT, unsigned short zone, float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float rotW) {
	std::stringstream str;
	str << "INSERT INTO `luni`.`world_objects` (`id`, `tempObjID`, `name`, `LOT`, `world`, `posX`, `posY`, `posZ`, `rotX`, `rotY`, `rotZ`, `rotW`) VALUES (NULL, '" << firstObjID << "', '" << UtfConverter::ToUtf8(name) << "', '" << LOT << "', '" << zone << "', '" << posX << "', '" << posY << "', '" << posZ << "', '" << rotX << "', '" << rotY << "', '" << rotZ << "', '" << rotW << "');";
	Database::Query(str.str());
	unsigned long long id = mysql_insert_id(Database::getConnection());
	return id;
}

void WorldObjectsTable::unregisterObject(unsigned long long id) {
	std::stringstream eqqr;
	eqqr << "DELETE FROM `world_objects` WHERE `id`='" << id << "';";
	Database::Query(eqqr.str());
}

std::vector<unsigned long long> WorldObjectsTable::getObjects(unsigned short zone) {
	std::vector<unsigned long long> query;
	std::stringstream str;
	str << "SELECT `id` FROM `world_objects` WHERE `world` = '" << zone << "';";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return query;
	}
	auto num = mysql_num_rows(qr);
	if (num == NULL || num == 0){
		return query;
	}
	else{
		query.reserve((unsigned int)num);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(qr)){
			query.push_back(std::stoll(row[0]));
		}
		return query;
	}
}

std::vector<unsigned long long> WorldObjectsTable::getObjects() {
	std::vector<unsigned long long> query;
	std::stringstream str;
	str << "SELECT `id` FROM `world_objects` WHERE 1;";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return query;
	}
	auto num = mysql_num_rows(qr);
	if (num == NULL || num == 0){
		return query;
	}
	else{
		query.reserve((unsigned int)num);
		MYSQL_ROW row;
		while (row = mysql_fetch_row(qr)){
			query.push_back(std::stoll(row[0]));
		}
		return query;
	}
}

unsigned long WorldObjectsTable::getLOT(unsigned long long id){
	std::stringstream str;
	str << "SELECT `LOT` FROM `world_objects` WHERE `id` = '" << id << "';";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return -1;
	}
	if (mysql_num_rows(qr) > 0){
		auto row = mysql_fetch_row(qr);
		unsigned long v = std::stoul(row[0]);
		return v;
	}
	return -1;
}

long long WorldObjectsTable::getTempObjID(unsigned long long id){
	std::stringstream str;
	str << "SELECT `tempObjID` FROM `world_objects` WHERE `id` = '" << id << "';";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return -1;
	}
	if (mysql_num_rows(qr) > 0){
		auto row = mysql_fetch_row(qr);
		long long v = std::stoll(row[0]);
		return v;
	}
	return -1;
}

unsigned long long WorldObjectsTable::getID(long long tempObjID){
	std::stringstream str;
	str << "SELECT `id` FROM `world_objects` WHERE `tempObjID` = '" << tempObjID << "';";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return -1;
	}
	if (mysql_num_rows(qr) > 0){
		auto row = mysql_fetch_row(qr);
		unsigned long long v = std::stoull(row[0]);
		return v;
	}
	return -1;
}

WorldObjectInfo WorldObjectsTable::getObjectInfo(unsigned long long id) {
	WorldObjectInfo objInfo;
	std::stringstream str;
	str << "SELECT `name`, `LOT`, `world`, `posX`, `posY`, `posZ`, `rotX`, `rotY`, `rotZ`, `rotW` FROM `world_objects` WHERE `id` = '" << id << "';";
	auto qr = Database::Query(str.str());
	if (qr == NULL){
		return objInfo;
	}
	if (mysql_num_rows(qr) > 0){
		auto row = mysql_fetch_row(qr);
		if (row[0] != NULL) {
			std::string str = row[0];
			objInfo.name = UtfConverter::FromUtf8(str);
		}
		objInfo.LOT = std::stoi(row[1]);
		objInfo.zone = std::stoi(row[2]);
		objInfo.posX = std::stof(row[3]);
		objInfo.posY = std::stof(row[4]);
		objInfo.posZ = std::stof(row[5]);
		objInfo.rotX = std::stof(row[6]);
		objInfo.rotY = std::stof(row[7]);
		objInfo.rotZ = std::stof(row[8]);
		objInfo.rotW = std::stof(row[9]);
		return objInfo;
	}
	return objInfo;
}

void WorldObjectsTable::updateTempObjID(unsigned long long id, long long tempObjID) {
	std::stringstream eqqr;
	eqqr << "UPDATE `world_objects` SET `tempObjID`='" << tempObjID << "' WHERE `id` = '" << id << "';";
	Database::Query(eqqr.str());
}