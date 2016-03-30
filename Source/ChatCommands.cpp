#include "ChatCommands.h"
#include "Social.h"
#include "WorldConnection.h"
#include "WorldServer.h"
#include "UtfConverter.h"
#include "Logger.h"
#include "World.h"
#include "GameMessages.h"
#include "ReplicaData.h"
#include "Worlds.h"
#include "Account.h"
#include "Packet.h"
#include "InventoryDB.h"
#include "PlayerObject.h"
#include "serverLoop.h"
#include "ReplicaComponents.h"
#include "pugixml.hpp"
#include "WorldObjectsDB.h"
#include "SQLiteDatabase.h"
#include "CDClientDB.h"
#include "Config.h"
#include "LUZFile.h"

#include <chrono>
#include <thread>

std::vector<ChatCommandHandler *> ChatCommandManager::ChatCommandHandlers;
std::unordered_map<std::wstring, ChatCommandHandler *> ChatCommandManager::ChatCommands;

void ChatCommandManager::registerCommands(ChatCommandHandler * CommandHandler){
	ChatCommandManager::ChatCommandHandlers.push_back(CommandHandler);
	std::vector<std::wstring> names = CommandHandler->getCommandNames();
	for (unsigned char k = 0; k < names.size(); k++){
		ChatCommandManager::ChatCommands.insert(std::make_pair(names.at(k), CommandHandler));
	}
}

void ChatCommandManager::handleCommand(std::wstring command, SessionInfo *s, std::vector<std::wstring> * params){
	if (command == L"help"){
		bool flag = true;
		if (params->size() > 0){
			std::wstring com = params->at(0);
			if (com.at(0) == L'/'){
				com = com.substr(1, com.size() - 1);
			}
			std::unordered_map<std::wstring, ChatCommandHandler *>::iterator it = ChatCommandManager::ChatCommands.find(com);
			if (it != ChatCommandManager::ChatCommands.end()){
				Chat::sendChatMessage(s->addr, L"Syntax: /" + com + L" " + it->second->getSyntax());
				Chat::sendChatMessage(s->addr, it->second->getDescription());
			}
			else{
				Chat::sendChatMessage(s->addr, L"Command " + params->at(0) + L" was not found");
			}
			flag = false;
		}
		if (flag){
			for (unsigned int k = 0; k < ChatCommandManager::ChatCommandHandlers.size(); k++){
				std::wstring desc = ChatCommandManager::ChatCommandHandlers.at(k)->getShortDescription();
				std::wstring cmd = ChatCommandManager::ChatCommandHandlers.at(k)->getCommandNames().at(0);
				unsigned long stringvalue = cmd.size() * 4;
				short diff = 0;
				std::wstring space = L"\t\t";
				for (unsigned int l = 0; l < cmd.size(); l++){
					wchar_t c = cmd.at(l);
					if (c == L'm') diff = diff + 2;
					else if (c == L'w') diff = diff + 2;
					else if (c == L'r') diff = diff - 1;
					else if (c == L'f') diff = diff - 2;
					else if (c == L't') diff = diff - 2;
					else if (c == L'i') diff = diff - 3;
					else if (c == L'j') diff = diff - 3;
					else if (c == L'l') diff = diff - 3;
				}

				if ((stringvalue + diff) > 30){
					space = L"\t";
				}
				
				if (desc != L""){
					Chat::sendChatMessage(s->addr, L"/" + cmd + space + desc);
				}
			}
		}
	}
	else{
		std::unordered_map<std::wstring, ChatCommandHandler *>::iterator it = ChatCommandManager::ChatCommands.find(command);
		if (it != ChatCommandManager::ChatCommands.end()){
			it->second->handleCommand(s, params);
		}
		else{
			Chat::sendChatMessage(s->addr, L"Command " + command + L" does not exist!");
		}
	}
}

void FlightCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	bool f2 = true;
	if (params->size() > 0){
		if (params->at(0) == L"off" || params->at(0) == L"false"){
			std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
			for (unsigned int k = 0; k < sessionsz.size(); k++){
				WorldServerPackets::SendGameMessage(sessionsz.at(k).addr, s->activeCharId, 561);
			}

			f2 = false;

			Chat::sendChatMessage(s->addr, L"Switched off fly mode!");
		}
	}
	if (f2){
		RakNet::BitStream *pc = WorldServerPackets::InitGameMessage(s->activeCharId, 561);

		pc->Write((unsigned long)0x70ba);
		pc->Write((unsigned short)0x8);
		pc->Write((unsigned char)0x5);
		pc->Write((unsigned char)0x2);
		pc->Write((unsigned short)0xc);
		pc->Write((unsigned char)0x3);
		pc->Write((unsigned short)0x6c1);
		pc->Write((unsigned char)0x0);
		pc->Write((unsigned char)0x1);
		pc->Write((unsigned char)0x80);
		pc->Write((unsigned char)0x7f);
		pc->Write((unsigned long)0xa7);

		std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
		for (unsigned int k = 0; k < sessionsz.size(); k++){
			WorldServer::sendPacket(pc, sessionsz.at(k).addr);
		}

		Chat::sendChatMessage(s->addr, L"Switched on fly mode!");
	}
}

std::vector<std::wstring> FlightCommandHandler::getCommandNames(){
	return { L"flight", L"jetpack", L"fly" };
}

std::wstring FlightCommandHandler::getDescription(){
	return L"Enables fly mode";
}

std::wstring FlightCommandHandler::getShortDescription(){
	return L"Enables fly mode";
}

std::wstring FlightCommandHandler::getSyntax(){
	return L"[off]";
}

void TeleportCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (params->size() == 3){
		std::string check = UtfConverter::ToUtf8(params->at(0));
		bool flag = isNumber(check);
		check = UtfConverter::ToUtf8(params->at(1));
		if (flag) flag = isNumber(check);
		check = UtfConverter::ToUtf8(params->at(2));
		if (flag) flag = isNumber(check);
		if (flag){
			RakNet::BitStream *bs = WorldServer::initPacket(RemoteConnection::CLIENT, ClientPacketID::SERVER_GAME_MSG);

			bs->Write(s->activeCharId);
			bs->Write((unsigned short)19);
			bs->Write(false);
			bs->Write(false);
			bs->Write(false);

			float x = 0.0F;
			float y = 0.0F;
			float z = 0.0F;

			if (params->size() > 2){
				x = std::stof(params->at(0));
				y = std::stof(params->at(1));
				z = std::stof(params->at(2));
			}

			bs->Write(x);
			bs->Write(y);
			bs->Write(z);

			std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
			for (unsigned int k = 0; k < sessionsz.size(); k++){
				WorldServer::sendPacket(bs, sessionsz.at(k).addr);
			}

			Chat::sendChatMessage(s->addr, L"Teleported!");
		}
		else{
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
	}
	else{
		Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
}

std::vector<std::wstring> TeleportCommandHandler::getCommandNames(){
	return{ L"teleport", L"tele", L"ot"};
}

std::wstring TeleportCommandHandler::getDescription(){
	return L"Teleports the player to a specific location of the map";
}
std::wstring TeleportCommandHandler::getShortDescription(){
	return L"Teleports player";
}
std::wstring TeleportCommandHandler::getSyntax(){
	return L"[<x> <y> <z>]";
}

void WhisperCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (params->size() > 1){
		std::wstring reciever = params->at(0);
		std::wstring message = params->at(1);

		for (unsigned int k = 2; k < params->size(); k++){
			message.append(L" ");
			message.append(params->at(k));
		}

		Logger::log("WRLD", "COMMAND", "Private message to " + UtfConverter::ToUtf8(reciever));
		Logger::log("WRLD", "COMMAND", UtfConverter::ToUtf8(message));
	}
}

std::vector<std::wstring> WhisperCommandHandler::getCommandNames(){
	return{ L"whisper", L"w", L"tell" };
}

std::wstring WhisperCommandHandler::getDescription(){
	return L"Sends another player a private chat message";
}
std::wstring WhisperCommandHandler::getShortDescription(){
	return L"Sends private message";
}
std::wstring WhisperCommandHandler::getSyntax(){
	return L"<reciever> <message>";
}

void TestmapCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (params->size() == 1){
		std::string check = UtfConverter::ToUtf8(params->at(0));
		bool flag = isNumber(check);
		if (flag){
			unsigned short argumentValue = std::stoi(params->at(0));
			ZoneId zone = static_cast<ZoneId>(argumentValue);
			Logger::log("WRLD", "CHAT", "Requesting teleport to " + std::to_string(argumentValue));
			if (getWorldTarget(zone).size() > 0){
				if (zone != ZoneId::NO_ZONE &&
					zone != ZoneId::KEELHAUL_CANYON &&
					zone != ZoneId::VERTIGO_LOOP_RACETRACK &&
					zone != ZoneId::DRAGONMAW_CHASM &&
					zone != ZoneId::AVANT_GARDENS_SURVIVAL){
					COMPONENT1_POSITION pos = getZoneSpawnPoint(zone, static_cast<ZoneId>(s->zone));
					if (zone != ZoneId::NO_ZONE) {
						WorldPlace place;

						place.zoneID = zone;
						place.mapClone = 0;
						place.mapInstance = 0;
						place.x = pos.x;
						place.y = pos.y;
						place.z = pos.z;

						CharactersTable::setCharactersPlace(s->activeCharId, place);
						// GameMSG::displayZoneSummary(s->activeCharId, false /*((char)std::to_string(argumentValue).at(2)) == '5'*/, false);

						Worlds::loadWorld(s->addr, zone, pos);

						Session::leave(s->activeCharId);
						ObjectsManager::clientLeaveWorld(s->activeCharId, s->addr);
					}
					else{
						Chat::sendChatMessage(s->addr, L"Cannot teleport to this zone!");
					}
				}
				else{
					Chat::sendChatMessage(s->addr, L"Cannot teleport to this zone!");
				}
			}
			else{
				Chat::sendChatMessage(s->addr, L"Cannot teleport to this zone!");
			}
		}
		else{
			Chat::sendChatMessage(s->addr, L"Cannot teleport to WorldID " + params->at(0));
		}
	}
	else{
		Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
}

std::vector<std::wstring> TestmapCommandHandler::getCommandNames(){
	return{ L"testmap", L"tp" };
}

std::wstring TestmapCommandHandler::getDescription(){
	return L"Sends the player to another world";
}
std::wstring TestmapCommandHandler::getShortDescription(){
	return L"Sends to another world";
}
std::wstring TestmapCommandHandler::getSyntax(){
	return L"<ZoneId>";
}

void SwitchCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 2) {
			std::string ip = UtfConverter::ToUtf8(params->at(0));
			short port = (short)std::stoi(params->at(1));

			SystemAddress target;
			target.SetBinaryAddress(ip.data());
			target.port = port;

			bool flag = Session::sendToInstance(s->addr, target);

			if (flag) {
				RakNet::BitStream * redirect = WorldServer::initPacket(RemoteConnection::CLIENT, ClientPacketID::SERVER_REDIRECT);
				PacketTools::WriteToPacket(redirect, ip, 33);
				redirect->Write(port);
				redirect->Write((unsigned char)1);
				WorldServer::sendPacket(redirect, s->addr);
			}
			else
				Chat::sendChatMessage(s->addr, L"Could not switch instance");
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
}

std::vector<std::wstring> SwitchCommandHandler::getCommandNames(){
	return{ L"switch", L"tpx" };
}

std::wstring SwitchCommandHandler::getDescription(){
	return L"Sends the player to another server";
}
std::wstring SwitchCommandHandler::getShortDescription(){
	return L"Sends to another server";
}
std::wstring SwitchCommandHandler::getSyntax(){
	return L"<IP> <port>";
}

void AddItemCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params) {
	if (params->size() > 1 && params->size() < 4) {
		std::string check = UtfConverter::ToUtf8(params->at(0));
		bool flag = isNumber(check);
		check = UtfConverter::ToUtf8(params->at(1));
		if (flag) flag = isNumber(check);
		if (flag){
			unsigned long LOT = std::stoi(params->at(0));
			unsigned long amount = std::stoi(params->at(1));

			if (CDClientDB::getObjectType(LOT) == "Loot" ||
				LOT == LOT::LOT_SLITHERSTRIKER ||
				LOT == LOT::LOT_NIGHTLASHER ||
				LOT == LOT::LOT_ENERGY_SPORK ||
				LOT == LOT::LOT_ZAPZAPPER ||
				AccountsTable::getRank(s->accountid) > 0) {
				unsigned long itemType = CDClientDB::getItemType(CDClientDB::getComponentID(LOT, 11));

				if (itemType == ItemType::HAIR ||
					itemType == ItemType::HAT ||
					itemType == ItemType::NECK ||
					itemType == ItemType::CHEST ||
					itemType == ItemType::LEGS ||
					itemType == ItemType::RIGHT_HAND ||
					itemType == ItemType::LEFT_HAND ||
					itemType == ItemType::CONSUMABLE ||
					AccountsTable::getRank(s->accountid) > 0) {

					long long charid = s->activeCharId;
					std::wstring person = L"your";
					long long objid = ObjectsTable::createObject(LOT);

					if (params->size() == 3) {
						long long buffer = CharactersTable::getObjidFromCharacter(UtfConverter::ToUtf8(params->at(2)));

						if (buffer > 0) {
							flag = true;
							charid = buffer;
							person = params->at(2) + L"'s";
						}
					}

					unsigned long slot = -1;
					for (int i = 0; (slot == -1) && (i != 128); i++){
						if (InventoryTable::getItemFromSlot(s->activeCharId, i) == -1)
							slot = i;
					}

					if (slot == -1)
						Chat::sendChatMessage(s->addr, L"Sorry, but " + person + L" inventory is full!");
					else {
						InventoryTable::insertItem(charid, objid, amount, slot, false);
						GameMSG::addItemToInventoryClientSync(charid, objid, LOT, slot, true);
						Chat::sendChatMessage(s->addr, L"Successfully added the requested item to " + person + L" inventory!");
					}
				}
				else
					Chat::sendChatMessage(s->addr, L"You aren't allowed to own this item!");
			}
			else
				Chat::sendChatMessage(s->addr, L"You aren't allowed to own this item!");
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
}

std::vector<std::wstring> AddItemCommandHandler::getCommandNames(){
	return{ L"gmadditem" };
}

std::wstring AddItemCommandHandler::getDescription(){
	return L"Adds an item to the users inventory";
}

std::wstring AddItemCommandHandler::getShortDescription(){
	return L"Adds an item";
}

std::wstring AddItemCommandHandler::getSyntax(){
	return L"<LOT> <amount> [<name>]";
}

void PositionCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);
	std::wstringstream wstr;
	if (player != NULL){
		COMPONENT1_POSITION pos = player->getComponent1()->getPosition();
		wstr << L"Position:  X:" << pos.x << "  Y:" << pos.y << "  Z:" << pos.z;
	}
	Chat::sendChatMessage(s->addr, wstr.str());
}

std::vector<std::wstring> PositionCommandHandler::getCommandNames(){
	return{ L"position", L"pos"};
}

std::wstring PositionCommandHandler::getDescription(){
	return L"Displays the current position of the player";
}

std::wstring PositionCommandHandler::getShortDescription(){
	return L"Displays position";
}

std::wstring PositionCommandHandler::getSyntax(){
	return L"";
}

void ClientCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){}

std::vector<std::wstring> ClientCommandHandler::getCommandNames(){
	return{ L"loc"};
}

std::wstring ClientCommandHandler::getDescription(){
	return L"";
}

std::wstring ClientCommandHandler::getShortDescription(){
	return L"";
}

std::wstring ClientCommandHandler::getSyntax(){
	return L"";
}

void AttributeCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (params->size() > 1){
		std::wstring attr = params->at(0);
		unsigned long value = std::stoul(params->at(1));
		bool max = false;
		float maxV = 0;
		if (params->size() > 2){
			max = true;
			maxV = std::stof(params->at(2));
		}
		PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);
		if (player != NULL){
			DestructibleComponent *c7 = player->getComponent7();
			COMPONENT7_DATA4 d4 = c7->getData4();
			if (attr == L"health"){
				d4.health = value;
				if (max) d4.maxHealth = maxV;
			}
			else if (attr == L"armor"){
				d4.armor = value;
				if (max) d4.maxArmor = maxV;
			}
			else if (attr == L"imagi"){
				d4.imagination = value;
				if (max) d4.maxImagination = maxV;
			}
			c7->setData4(d4);
			ObjectsManager::serialize(player);
		}
	}
}

std::vector<std::wstring> AttributeCommandHandler::getCommandNames(){
	return{ L"setattr" };
}

std::wstring AttributeCommandHandler::getDescription(){
	return L"Sets attributes for the player";
}

std::wstring AttributeCommandHandler::getShortDescription(){
	return L"Sets attributes";
}

std::wstring AttributeCommandHandler::getSyntax(){
	return L"(health|armor|imagi) <value> [<max>]";
}

void PacketCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0){
		std::wstring msg = L"Usage: /packet <path>";
		if (params->size() > 0){
			std::vector<unsigned char> v = OpenPacket(UtfConverter::ToUtf8(params->at(0)));
			if (v.size() > 0){
				WorldServer::sendPacket(v, s->addr);
				msg = L"Success sending packet";
			}
			else{
				msg = L"Error sending packet";
			}
		}
		Chat::sendChatMessage(s->addr, msg, L"System");
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> PacketCommandHandler::getCommandNames(){
	return{ L"packet" };
}

std::wstring PacketCommandHandler::getDescription(){
	return L"Sends a packet from disk";
}

std::wstring PacketCommandHandler::getShortDescription(){
	return L"Sends packet";
}

std::wstring PacketCommandHandler::getSyntax(){
	return L"<file>";
}

void AnnouncementCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0){
		if (params->size() > 1){
			long long charid;
			std::wstring rec = params->at(0);
			bool flagOne = false;
			if (rec == L"#"){
				charid = s->activeCharId;
				flagOne = true;
			}
			else if (rec == L"*"){
				std::string msg = UtfConverter::ToUtf8(params->at(1));
				std::string title = "Information";
				if (params->size() > 2){
					title = UtfConverter::ToUtf8(params->at(2));
				}
				std::vector<SessionInfo> wsessions = SessionsTable::getClientsInWorld(s->zone);
				for (unsigned int i = 0; i < wsessions.size(); i++){
					Chat::sendMythranInfo(wsessions.at(i).activeCharId, msg, title);
				}
			}
			else{
				charid = CharactersTable::getObjidFromCharacter(UtfConverter::ToUtf8(rec));
				flagOne = true;
			}

			if (flagOne){
				if (charid > 0){
					std::string msg = UtfConverter::ToUtf8(params->at(1));
					std::string title = "Information";
					if (params->size() > 2){
						title = UtfConverter::ToUtf8(params->at(2));
					}
					Chat::sendMythranInfo(charid, msg, title);
				}
				else{
					std::wstring response = L"\"" + rec + L"\" is not a valid Playername";
					Chat::sendChatMessage(s->addr, response);
				}
			}
		}
		else{
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
	}
	else{
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
	}
}

std::vector<std::wstring> AnnouncementCommandHandler::getCommandNames(){
	return{ L"announce", L"popup" };
}

std::wstring AnnouncementCommandHandler::getDescription(){
	return L"Sends a localized Mythran announcement";
}

std::wstring AnnouncementCommandHandler::getShortDescription(){
	return L"Sends announcement";
}

std::wstring AnnouncementCommandHandler::getSyntax(){
	return L"(<playername>|*|#) <message> [<title>]";
}

void AdminCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (params->size() == 1){
		ListCharacterInfo cinfo = CharactersTable::getCharacterInfo(s->activeCharId);

		unsigned short maxLevel = AccountsTable::getRank(s->accountid);
		unsigned short oldLevel = cinfo.info.gmlevel;
		std::string check = UtfConverter::ToUtf8(params->at(0));
		bool flag = isNumber(check);
		if (flag){
			unsigned short newLevel = (unsigned char)std::stoi(params->at(0));
			unsigned char success = 0;
			if (newLevel <= maxLevel){
				success = 1;
			}
			else{
				newLevel = oldLevel;
			}

			if (success == 1) CharactersTable::setGMlevel(s->activeCharId, newLevel);

			RakNet::BitStream * packet = WorldServer::initPacket(RemoteConnection::CLIENT, 16);
			packet->Write(success);
			packet->Write(maxLevel);
			packet->Write(oldLevel);
			packet->Write(newLevel);
			WorldServer::sendPacket(packet, s->addr);
		}
		else{
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
	}
	else{
		Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
}

std::vector<std::wstring> AdminCommandHandler::getCommandNames(){
	return{ L"gmlevel" };
}

std::wstring AdminCommandHandler::getDescription(){
	return L"Sets your GM-Level";
}

std::wstring AdminCommandHandler::getShortDescription(){
	return L"Sets GM-Level";
}

std::wstring AdminCommandHandler::getSyntax(){
	return L"<level>";
}

void SpawnObjectCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params) {
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 1 || params->size() == 2) {
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag){
				unsigned long LOT = std::stoi(params->at(0));
				std::string type = CDClientDB::getObjectType(LOT);

				std::wstring name = L"";
				if (params->size() > 1)
					name = params->at(1);

				if (type == "NPC" || type == "UserGeneratedNPCs") {
					PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);

					ControllablePhysicsComponent *c1 = player->getComponent1();
					COMPONENT1_POSITION position = c1->getPosition();
					COMPONENT1_ROTATION rotation = c1->getRotation();

					World *world = new World(player->world.zone, 0, 0);

					NPCObject *npc = new NPCObject(LOT, world->zone);
					npc->name = name;
					npc->setPosition(position.x, position.y, position.z);
					npc->setRotation(rotation.x, rotation.y, rotation.z, rotation.w);

					unsigned long long id = WorldObjectsTable::registerObject(npc->name, npc->objid, npc->LOT, npc->world.zone, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, rotation.w);
					ObjectsManager::registerObject(npc);
					ObjectsManager::create(npc);

					Chat::sendChatMessage(s->addr, L"Successfully spawned object with LOT '" + std::to_wstring(npc->getLOT()) + L"' in your world and saved it with ID '" + std::to_wstring(id) + L"'!");
				}

				else if (type == "Environmental" || type == "MovingPlatforms" || type == "LEGO brick" || type == "Loot" || type == "Smashables" || type == "Rebuildables" || type == "LUP") {
					PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);

					ControllablePhysicsComponent *c1 = player->getComponent1();
					COMPONENT1_POSITION position = c1->getPosition();
					COMPONENT1_ROTATION rotation = c1->getRotation();

					World *world = new World(player->world.zone, 0, 0);

					EnvironmentalObject *environmental = new EnvironmentalObject(LOT, world->zone);
					environmental->setPosition(position.x, position.y, position.z);
					environmental->setRotation(rotation.x, rotation.y, rotation.z, rotation.w);

					unsigned long long id = WorldObjectsTable::registerObject(environmental->name, environmental->objid, environmental->LOT, environmental->world.zone, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, rotation.w);
					ObjectsManager::registerObject(environmental);
					ObjectsManager::create(environmental);

					Chat::sendChatMessage(s->addr, L"Successfully spawned object with LOT '" + std::to_wstring(environmental->getLOT()) + L"' in your world and saved it with ID '" + std::to_wstring(id) + L"'!");
				}

				else if (type == "Enemies") {
					PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);

					ControllablePhysicsComponent *c1 = player->getComponent1();
					COMPONENT1_POSITION position = c1->getPosition();
					COMPONENT1_ROTATION rotation = c1->getRotation();

					World *world = new World(player->world.zone, 0, 0);

					EnemyObject *enemies = new EnemyObject(LOT, world->zone);
					enemies->name = name;
					enemies->setPosition(position.x, position.y, position.z);
					enemies->setRotation(rotation.x, rotation.y, rotation.z, rotation.w);

					unsigned long long id = WorldObjectsTable::registerObject(enemies->name, enemies->objid, enemies->LOT, enemies->world.zone, position.x, position.y, position.z, rotation.x, rotation.y, rotation.z, rotation.w);
					ObjectsManager::registerObject(enemies);
					ObjectsManager::create(enemies);

					Chat::sendChatMessage(s->addr, L"Successfully spawned object with LOT '" + std::to_wstring(enemies->getLOT()) + L"' in your world and saved it with ID '" + std::to_wstring(id) + L"'!");
				}

				else
					Chat::sendChatMessage(s->addr, L"Invalid LOT: '" + std::to_wstring(LOT) + L"'");
			}
			else
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> SpawnObjectCommandHandler::getCommandNames() {
	return{ L"spawnobject" };
}

std::wstring SpawnObjectCommandHandler::getDescription() {
	return L"Spawns an object by LOT";
}

std::wstring SpawnObjectCommandHandler::getShortDescription() {
	return L"Spawns object";
}

std::wstring SpawnObjectCommandHandler::getSyntax() {
	return L"<LOT> [<name>]";
}

void DeleteObjectCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params) {
	if (AccountsTable::getRank(s->accountid) > 0){
		if (params->size() == 1) {
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag){
				unsigned long long id = std::stoull(params->at(0));

				long long objID = WorldObjectsTable::getTempObjID(id);
				if (objID != -1){
					ReplicaObject *obj = ObjectsManager::getObjectByID(objID);

					if (NPCObject *npc_obj = dynamic_cast<NPCObject*>(obj)) {
						npc_obj->deleteSubObjects();
					}

					if (EnemyObject *enemies_obj = dynamic_cast<EnemyObject*>(obj)) {
						enemies_obj->deleteSubObjects();
					}

					WorldObjectsTable::unregisterObject(id);
					ObjectsManager::unregisterObject(obj);
					ObjectsManager::destruct(obj);

					Chat::sendChatMessage(s->addr, L"Successfully deleted object with ID '" + std::to_wstring(id) + L"'!");
				}
				else{
					Chat::sendChatMessage(s->addr, L"There isn't an object with ID '" + std::to_wstring(id) + L"'!");
				}
			}
			else{
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
			}
		}
		else{
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
	}
	else {
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
	}
}

std::vector<std::wstring> DeleteObjectCommandHandler::getCommandNames() {
	return{ L"deleteobject" };
}

std::wstring DeleteObjectCommandHandler::getDescription() {
	return L"Deletes an object by ID";
}

std::wstring DeleteObjectCommandHandler::getShortDescription() {
	return L"Deletes object";
}

std::wstring DeleteObjectCommandHandler::getSyntax() {
	return L"<ID>";
}

void NearMeCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params) {
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 1) {
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag) {
				long radius = std::stol(params->at(0));

				PlayerObject *player = (PlayerObject *)ObjectsManager::getObjectByID(s->activeCharId);
				std::vector<unsigned long long> sameWorld = WorldObjectsTable::getObjects(player->world.zone);

				if (sameWorld.size() > 0) {
					std::vector<long long> nearChar;

					ControllablePhysicsComponent *c1 = player->getComponent1();
					COMPONENT1_POSITION position = c1->getPosition();

					float posX_m_r = position.x - radius;
					float posX_p_r = position.x + radius;

					float posY_m_r = position.y - radius;
					float posY_p_r = position.y + radius;

					float posZ_m_r = position.z - radius;
					float posZ_p_r = position.z + radius;

					for (uint i = 0; i < sameWorld.size(); i++) {
						WorldObjectInfo objInfo = WorldObjectsTable::getObjectInfo(sameWorld.at(i));
						if ((objInfo.posX >= posX_m_r && objInfo.posX <= posX_p_r) && (objInfo.posY >= posY_m_r && objInfo.posY <= posY_p_r) && (objInfo.posZ >= posZ_m_r && objInfo.posZ <= posZ_p_r))
							nearChar.push_back(sameWorld.at(i));
					}

					if (nearChar.size() > 0) {
						std::wstringstream wss;
						wss << L"Found the following world objects within a radius of '";
						wss << radius;
						wss << L"':\n";

						for (uint i = 0; i < nearChar.size(); i++) {
							std::wstring type = UtfConverter::FromUtf8(CDClientDB::getObjectType(WorldObjectsTable::getLOT(nearChar.at(i))));
							if (type == L"Enemies")
								type = L"Enemy";
							else if (endsWith(type, L"s"))
								type = removeRight(type, 1);

							wss << L"- ";
							wss << type;
							wss << L" with ID '";
							wss << nearChar.at(i);
							wss << L"' and LOT '";
							wss << WorldObjectsTable::getLOT(nearChar.at(i));
							wss << L"'";

							if (i != nearChar.size() - 1)
								wss << L"\n";
						}

						Chat::sendChatMessage(s->addr, wss.str());
					}
					else
						Chat::sendChatMessage(s->addr, L"There aren't any world objects near you! (At least not within a radius of '" + std::to_wstring(radius) + L"'!)");
				}
				else
					Chat::sendChatMessage(s->addr, L"There aren't any world objects in your world!");
			}
			else
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> NearMeCommandHandler::getCommandNames() {
	return{ L"nearme" };
}

std::wstring NearMeCommandHandler::getDescription() {
	return L"Shows a list of world objects near you in a specific radius";
}

std::wstring NearMeCommandHandler::getShortDescription() {
	return L"Lists up objects near you";
}

std::wstring NearMeCommandHandler::getSyntax() {
	return L"<radius>";
}

void DebugCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 1){
		// Normally used for debugging stuff
	}
}

std::vector<std::wstring> DebugCommandHandler::getCommandNames(){
	return{ L"debug" };
}

std::wstring DebugCommandHandler::getDescription(){
	return L"[NULL]";
}

std::wstring DebugCommandHandler::getShortDescription(){
	return L"[NULL]";
}

std::wstring DebugCommandHandler::getSyntax(){
	return L"";
}

void SetMoneyCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0){
		if (params->size() > 0 && params->size() < 3){
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag){
				long long currency = std::stoll(params->at(0));
				long long objid = s->activeCharId;
				flag = false;

				std::string person = "your";

				if (params->size() == 2){
					if (params->at(1) == L"*"){
						person = "everyone's";
						flag = true;
					}
					else{
						long long i = CharactersTable::getObjidFromCharacter(UtfConverter::ToUtf8(params->at(1)));
						if (i > 0){
							objid = i;
							person = UtfConverter::ToUtf8(params->at(1));
							person.append("'s");
						}
					}
				}

				if (flag){
					std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
					for (unsigned int k = 0; k < sessionsz.size(); k++){
						CharactersTable::setCharacterMoney(sessionsz.at(k).activeCharId, currency);
					}
				}
				else
					CharactersTable::setCharacterMoney(objid, currency);

				Chat::sendChatMessage(s->addr, L"Successfully updated " + UtfConverter::FromUtf8(person) + L" currency!");
			}
			else{
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
			}
		}
		else{
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
	}
	else {
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
	}
}

std::vector<std::wstring> SetMoneyCommandHandler::getCommandNames(){
	return{ L"setmoney" };
}

std::wstring SetMoneyCommandHandler::getDescription(){
	return L"Sets the currency of the specified player";
}

std::wstring SetMoneyCommandHandler::getShortDescription(){
	return L"Sets currency";
}

std::wstring SetMoneyCommandHandler::getSyntax(){
	return L"<amount> [<name>]";
}

void DanceCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0 && params->size() == 1 && params->at(0) == L"*") {
		std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
		for (unsigned int k = 0; k < sessionsz.size(); k++){
			GameMSG::playAnimation(sessionsz.at(k).activeCharId, L"jig-loop");
		}
	}
	else
		GameMSG::playAnimation(s->activeCharId, L"jig-loop");
}

std::vector<std::wstring> DanceCommandHandler::getCommandNames(){
	return{ L"dance", L"d" };
}

std::wstring DanceCommandHandler::getDescription(){
	return L"DANCE!";
}

std::wstring DanceCommandHandler::getShortDescription(){
	return L"DANCE!";
}

std::wstring DanceCommandHandler::getSyntax(){
	return L"[*]";
}

void SetNameCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 2) {
			std::string currentName = UtfConverter::ToUtf8(params->at(0));
			std::string newName = UtfConverter::ToUtf8(params->at(1));

			ListCharacterInfo info = CharactersTable::getCharacterInfo(currentName);
			if (info.info.objid != 0) {
				CharactersTable::setCharacterName(info.info.objid, newName);

				Chat::sendMythranInfo(info.info.objid, "Your name has been changed from '" + currentName + "' to '" + newName + "'!", "Your name has been changed!");

				SystemAddress addr = SessionsTable::findCharacter(info.info.objid);
				if (addr != UNASSIGNED_SYSTEM_ADDRESS)
					GameMSG::setName(info.info.objid, params->at(1));

				Chat::sendChatMessage(s->addr, L"Successfully changed " + params->at(0) + L"'s name to " + params->at(1) + L"!");
			}
			else
				Chat::sendChatMessage(s->addr, L"Player not found!");
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> SetNameCommandHandler::getCommandNames(){
	return{ L"setname" };
}

std::wstring SetNameCommandHandler::getDescription(){
	return L"Sets the name of the specified player";
}

std::wstring SetNameCommandHandler::getShortDescription(){
	return L"Sets a player's name";
}

std::wstring SetNameCommandHandler::getSyntax(){
	return L"<current name> <new name>";
}

void PlayAnimationCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 1 || params->size() == 2) {
			std::wstring animationID = params->at(0);
			if (CDClientDB::isAnimationValid(UtfConverter::ToUtf8(animationID))) {
				if (params->size() == 2 && params->at(1) == L"*") {
					std::vector<SessionInfo> sessionsz = SessionsTable::getClientsInWorld(s->zone);
					for (unsigned int k = 0; k < sessionsz.size(); k++){
						GameMSG::playAnimation(sessionsz.at(k).activeCharId, animationID);
					}
				}
				else
					GameMSG::playAnimation(s->activeCharId, animationID);

				Chat::sendChatMessage(s->addr, L"Animation '" + animationID + L"' is playing!");
			}
			else
				Chat::sendChatMessage(s->addr, L"Invalid AnimationID!");
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> PlayAnimationCommandHandler::getCommandNames(){
	return{ L"playanimation", L"pa" };
}

std::wstring PlayAnimationCommandHandler::getDescription(){
	return L"Plays the specified animation";
}

std::wstring PlayAnimationCommandHandler::getShortDescription(){
	return L"Plays an animation";
}

std::wstring PlayAnimationCommandHandler::getSyntax(){
	return L"<animationID> [*]";
}

void EquipOfCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params){
	if (AccountsTable::getRank(s->accountid) > 0) {
		if (params->size() == 1) {
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag){
				unsigned long LOT = std::stoul(params->at(0));

				unsigned long componentID = CDClientDB::getComponentID(LOT, 17);
				if (componentID != -1) {
					std::vector<unsigned long> equipment = CDClientDB::getInventoryComponentData(LOT);

					if (equipment.size() > 0) {
						for (int i = 0; i < equipment.size(); i++) {
							unsigned long slot = -1;
							for (int k = 0; (slot == -1) && (k != 128); k++){
								if (InventoryTable::getItemFromSlot(s->activeCharId, k) == -1)
									slot = k;
							}

							if (slot == -1)
								Chat::sendChatMessage(s->addr, L"Sorry, but your inventory is full!");
							else {
								long long objID = ObjectsTable::createObject(equipment.at(i));

								InventoryTable::insertItem(s->activeCharId, objID, 1, slot, false);
								GameMSG::addItemToInventoryClientSync(s->activeCharId, objID, equipment.at(i), slot, true);
								Chat::sendChatMessage(s->addr, L"Successfully added item with LOT " + std::to_wstring(equipment.at(i)) + L" to your inventory!");
							}
						}
					}
					else
						Chat::sendChatMessage(s->addr, L"The InventoryComponent of LOT " + std::to_wstring(LOT) + L" seems to be empty!");
				}
				else
					Chat::sendChatMessage(s->addr, L"The InventoryComponent of LOT " + std::to_wstring(LOT) + L" seems to be empty!");
			}
			else
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> EquipOfCommandHandler::getCommandNames(){
	return{ L"equipof", L"eqo" };
}

std::wstring EquipOfCommandHandler::getDescription(){
	return L"Adds the equipment of a specified NPC to your inventory";
}

std::wstring EquipOfCommandHandler::getShortDescription(){
	return L"Gives items of an NPC";
}

std::wstring EquipOfCommandHandler::getSyntax(){
	return L"<animationID> [*]";
}

void TestSmashCommandHandler::handleCommand(SessionInfo *s, std::vector<std::wstring> * params) {
	if (AccountsTable::getRank(s->accountid) > 0){
		if (params->size() == 1) {
			std::string check = UtfConverter::ToUtf8(params->at(0));
			bool flag = isNumber(check);
			if (flag){
				unsigned long long id = std::stoull(params->at(0));

				long long objID = WorldObjectsTable::getTempObjID(id);
				if (objID != -1){
					ReplicaObject *obj = ObjectsManager::getObjectByID(objID);

					if (obj != NULL) {
						CharactersTable::killCharacter(objID);
						Chat::sendChatMessage(s->addr, L"Done!");
					}
					else
						Chat::sendChatMessage(s->addr, L"There isn't an object with ID '" + std::to_wstring(id) + L"'!");
				}
				else
					Chat::sendChatMessage(s->addr, L"There isn't an object with ID '" + std::to_wstring(id) + L"'!");
			}
			else
				Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
		}
		else
			Chat::sendChatMessage(s->addr, L"Syntax: /" + this->getCommandNames().at(0) + L" " + this->getSyntax());
	}
	else
		Chat::sendChatMessage(s->addr, L"You don't have permission to use this command!");
}

std::vector<std::wstring> TestSmashCommandHandler::getCommandNames() {
	return{ L"testsmash", L"smash" };
}

std::wstring TestSmashCommandHandler::getDescription() {
	return L"SMASHING TEST";
}

std::wstring TestSmashCommandHandler::getShortDescription() {
	return L"SMASHING TEST";
}

std::wstring TestSmashCommandHandler::getSyntax() {
	return L"<ID>";
}