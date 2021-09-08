// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include "Server.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#include <vector>
#include <string.h>
#include <sstream>
#include <ctype.h>
#include <ctime>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment (lib,"ws2_32.lib")

using namespace std;

// TODO: reference additional headers your program requires here

// message format from client send to server
const string FORMAT_LOGIN = "USER";
const string FORMAT_POST_MESS = "POST";
const string FORMAT_LOGOUT = "QUIT";

// code respone from function log in
const int CODE_LOGIN_SUCCESS = 10;
const int CODE_LOGIN_FAIL_NOT_EXIST_ACCOUNT = 11;
const int CODE_LOGIN_FAIL_ACCOUNT_BLOCK = 12;
const int CODE_LOGIN_FAIL_ALREADY_SIGNED = 13;

// code respone from function post message
const int CODE_POST_SUCCESS = 20;
const int CODE_POST_FAIL_NOT_SINGED = 21;

// code respone from function log out
const int CODE_LOGOUT_SUCCESS = 30;
const int CODE_LOGOUT_FAIL_NOT_SINGED = 31;

// code packet error from client
const int CODE_PACKET_RECEIVE_ERROR = -40;

// another code from server, default is system error
const int CODE_ERROR_SYSTEM_DEFAULT = 500;

const string SERVER_ADDR = "127.0.0.1"; // address server
const int SERVER_PORT = 6000; // server port

const int MAX_CLIENT = 1024;
const int HEADER_SIZE = 10; // transactionID_size and data_size at the top of the packet send to server
const int PADDING_BUFF = 10;

const string ACCOUNT_FILE = "account.txt"; // please put the account.txt file in the folder Debug
const string LOG_20183966 = "log_20183966.txt";

const string DELIMITER = "\r\n";