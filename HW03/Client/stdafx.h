// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"
#include <iostream>
#include <fstream>
#include <conio.h>
#include <stdio.h>
#include <vector>
#include <stdlib.h> 
#include <string.h>
#include <sstream>
#include <ctype.h>
#include <ctime>
#include <process.h>
#include <errno.h> 
#include <sys/types.h> 
#include <tchar.h>
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
const int CODE_LOGIN_FAIL_SIGNED_ANOTHER_DEVICE = 14;

// code respone from function post message
const int CODE_POST_SUCCESS = 20;
const int CODE_POST_FAIL_NOT_SINGED = 21;

// code respone from function log out
const int CODE_LOGOUT_SUCCESS = 30;
const int CODE_LOGOUT_FAIL_NOT_SINGED = 31;

// another code from server, default is system error
const int CODE_ERROR_SYSTEM_DEFAULT = 500;

const int HEADER_SIZE = 10; // transactionID_size and data_size at the top of the packet send to server
const int PADDING_BUFF = 10;

const string DELIMITER = "\r\n";