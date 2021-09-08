#pragma once

#include <iostream>
#include <fstream>
#include <string.h>
#include <sstream>
#include <winsock2.h>
#include <WS2tcpip.h>

#include "utils.h"
#include "constant.h"
#include "message.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

// TODO: reference additional headers your program requires here

// define struct for client when connect with server
typedef struct client_session_management {
	bool free;				// client is free or not
	SOCKET socket;			// socket connect of client
	int clientPort;			// port of client
	char* clientIP;			// ip address of client
	bool isLogin;			// client is login or not
	string username;		// username of account login 
	string rootPath;		// root path folder of account login
	string currentPath;		// current path folder of user access
	int mode;				// mode of client (tranfser file or message)

    // default constructor
	client_session_management() {
		this->free = true;
		this->socket = NULL;
		this->clientPort = -1;
		this->clientIP = NULL;
		this->isLogin = false;
		this->username = "";
		this->rootPath = "";
		this->currentPath = "";
		this->mode = NORMAL_TASK;
	}

	// constructor all fields
	client_session_management(bool _free, SOCKET _socket, int _clientPort, char* _clientIP,
		bool _isLogin, string _username, string _rootPath, string _currentPath, int _mode) {
		this->free = _free;
		this->socket = _socket;
		this->clientPort = _clientPort;
		this->clientIP = _clientIP;
		this->isLogin = _isLogin;
		this->username = _username;
		this->rootPath = _rootPath;
		this->currentPath = _currentPath;
		this->mode = _mode;
	}
} CLIENT;


/**
* @function logManager: record request from client
*
* @param1 client: client send request to server
* @param2 msg: the message of request from client
* @param3 rs: result from server send to client after processing request
*
**/
void logManager(CLIENT *client, Message msg, int rs) {
	stringstream log;
	log << client->clientIP << ":" << client->clientPort << " " << getCurrentTimeInSystem()
		<< " $ " << (client->isLogin ? client->username : "Not Login") << " $ " << msg.opcode << " # " << msg.payload << " $ " << rs;

	ofstream logFile;
	logFile.open(LOG_SYSTEM, ios::app);
	logFile << log.str() << endl;
	logFile.close();
}