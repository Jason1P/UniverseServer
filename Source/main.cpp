/*
	Lego Universe Server WIP
	Code done by humanoid24 (aka Triver) as of 01.12.2013

	Additional credits to raffa505 30.01.2013 - 14.02.2013 (Database MySQL, Character, User, World, Color, Threads, Loading both Nimbus 
	Isle and Nimbus Station)

	Added code and additional documentation done by Jon002 20.02.2015 (Character Creation, Custom Logins and success states, Dynamic Packets
	for Auth and Char servers)

	"SCREW THESE CREDITS" by Jonny (a.k.a CuzItsJonny)

	This source code requires RakNet version 3.25 as an external dependency to work with the LU Client.

	The source is open and free under the GPL License, Version 3 for use on a non-commercial basis
*/

#include "serverLoop.h"
#include "Database.h"
#include "AccountsDB.h"
#include "FileConfiguration.h"
#include "ServerDB.h"
#include "Config.h"

#include <sys/stat.h>
#include <conio.h>
#include "Logger.h"

#include <sstream>
#include <iostream>
#include <windows.h>

#ifdef _WIN32
	// For mkdir command
	#include <direct.h>
#else
	#include <sys/stat.h>
#endif

void ConsoleLoop(){
	// If quit is ever equal to true, quit the server
	bool quit = false;

	// Keep the server from quitting by using a infinite loop
	while (!quit) {
		//if (_kbhit()) { // Parsing server commands. Press enter to start writing a command (may need to lock consoleoutmutex...)
			std::string rawCommand; // Initialize command string...
			
			std::cout << "> "; // Print "> " to show user where to type
			std::cin >> rawCommand; // Get the command

			std::vector<std::string> command = split(rawCommand, ' ');

			// Match command to a pre-specified command here...
			if (command[0] == "help") {
				std::stringstream str;
				str << "Available commands:" << std::endl <<
					"quit        = Quit the Server" << std::endl <<
					"register    = Register New User" << std::endl <<
					"sessions    = Show Number of sessions" << std::endl <<
					"envcheck    = Checks environment" << std::endl;
				std::cout << str.str();
			}
			else if (command[0] == "quit") quit = LUNIterminate = true;
			else if (command[0] == "register") {
				std::string username, password;
				std::cout << "Username: ";
				std::cin >> username; // Get the username
				if (AccountsTable::getAccountID(username) == 0) { // Check to see if username already exists... If not, continue
					std::cout << "Password: ";
					std::cin >> password; // Get the password
					// Create the new user into the database
					unsigned long long acid = AccountsTable::addAccount(username, password);
					if (acid > 0){
						std::stringstream str;
						str << "Account for '" << username << "' has been created with id " << acid << std::endl;
						std::cout << str.str();
					}
				}
				else std::cout << "Username already exist!\n";
			}
			else if (command[0] == "sessions"){
				std::cout << "Current session count: " << std::to_string(SessionsTable::count()) << std::endl;
			}
			else std::cout << "Invalid Command: " << command[0] << "!" << std::endl;
		//}
	}
}

void inset(unsigned char color){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	std::cout << "   ";
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void color(unsigned char color){
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

// This is the entry point into the server (the main() function)
int main(int argc, char* argv[]) {
	// Print starting info to the console
	std::cout << std::endl << std::endl;
	inset(207);  std::cout << "                                      " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GG     GG   OO  " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GGG    GG   OO  " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GGGG   GG   OO  " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GG GG  GG   OO  " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GG  GG GG   OO  " << std::endl;
	inset(207);  std::cout << "  LL       EE    EE   GG   GGGG   OO  " << std::endl;
	inset(207);  std::cout << "  LLLLLLL   EEEEEE    GG    GGG   OO  " << std::endl;
	inset(207);  std::cout << "                                      " << std::endl;
	inset(207);  std::cout << "  Custom   LEGO    Universe   Server  " << std::endl;
	inset(207);  std::cout << "                                      " << std::endl;
	std::cout << std::endl << std::endl;
	inset(15);  std::cout << "--------------------------------------" << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "Original project by ";
	color(13); std::cout << "raffa505";
	color(15); std::cout << ": " << std::endl;
	inset(11); std::cout << "luniserver.sf.net" << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "LUNIServer Github:" << std::endl;
	inset(11); std::cout << "github.com/LUNIServer/UniverseServer" << std::endl;
	inset(15); std::cout << std::endl;
	inset(15); std::cout << "This software  is still  very unstable" << std::endl;
	inset(15); std::cout << "and  may  crash  at  any  time. Please" << std::endl;
	inset(15); std::cout << "report any unreported issues on Github" << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "--------------------------------------" << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "LUNI is licensed under the AGPLv3:" << std::endl;
	inset(11); std::cout << "www.gnu.org/licenses/agpl-3.0.html  " << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "The  LEGO Group  has  not  endorsed or" << std::endl;
	inset(15); std::cout << "authorized the  operation of this game" << std::endl;
	inset(15); std::cout << "and  is  not  liable  for  any  safety" << std::endl;
	inset(15); std::cout << "issues in relation to the operation of" << std::endl;
	inset(15); std::cout << "game." << std::endl;
	std::cout << std::endl;
	inset(15); std::cout << "--------------------------------------" << std::endl;
	color(7); std::cout << std::endl << std::endl;
	
	std::cout << " Server Log" << std::endl;
	std::cout << "-----------------------------------------" << std::endl;
	Logger::setLogFile("server.log");
	Logger::log("MAIN", "", "Running update \"27.11.15_1\" | Initializing LUNI test server...\n");

	// Args parser
	int state = 0;
	std::string setting = "";
	ServerRole Role = ROLE_CONSOLE;

	for (int argi = 0; argi < argc; argi++){
		std::string arg = std::string(argv[argi]);
		Logger::log("MAIN", "ARGS", arg, LOG_ALL);
		switch (state){
		case 2:
			{
				bool f = true;
				if (arg.size() > 1){
					if (arg.substr(0, 2) == "--"){
						f = false;
					}
				}
				if (f){
					setting = arg;
					break;
				}
			}
		case 0:
			if (arg == "--console"){
				Role = ROLE_CONSOLE;
			}
			if (arg == "--gConfig"){
				Config::generateDefaultConfig();
				Role = ROLE_CONSOLE;
			}
			if (arg == "--world"){
				Role = ROLE_WORLD;
				setting = "World";
				state = 2;
			}
			if (arg == "--auth"){
				Role = ROLE_AUTH;
				setting = "Auth";
				state = 2;
			}
			if (arg == "--chat"){
				Role = ROLE_CHAT;
				setting = "Chat";
				state = 2;
			}
			break;
		}
	}

	Database::Init(Config::getMySQLHost(), Config::getMySQLDatabase(), Config::getMySQLUsername(), Config::getMySQLPassword());
	unsigned int db_connect_result = Database::Connect();
	if (db_connect_result > 0){
		QuitError("Could not connect to MYSQL database");
	}
	Logger::log("MAIN", "CONFIG", "Connected to mysql database!");

	// Create the directory .//logs// if it does not exist
	_mkdir("logs");

	// If debug is on, print additional data to the console
	#ifdef DEBUG
	{
		// Setting this to LOG_NORMAL is done on purpose to avoid confusion
		// when activeLogLevel is below DEBUG but DEBUG is actually on
		Logger::log("MAIN", "DEBUG", "is ON!", LOG_NORMAL);
	}
	#endif

	// Start the two new threads (Auth and World servers)
	if (Role == ROLE_AUTH){
		AuthLoop();
	}
	else if (Role == ROLE_WORLD){
		WorldLoop();
	}
	/*else if (Role == ROLE_CHAT){
		ChatLoop();
	}*/
	else if (Role == ROLE_CONSOLE){
		ConsoleLoop();
	}

	exit(0);
}