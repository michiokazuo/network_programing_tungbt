// TCP_Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SHUT_DOWN ""

vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

/**
* @function recvDataFromServer: print data receive from server to console
*
* @param data: the string receive from server
*
**/
void recvDataFromServer(string data) {
	cout << data << endl;
}

int main(int argc, char *argv[])
{
	// Step 1: Init Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		cout << "Winsock 2.2 is not supported!!!" << endl;
		return -1;
	}

	// Step 2: Construct socket
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	cout << "Client started!" << endl;

	// (optional) Set time-out for receiving
	//int tv = 10000; // Time-out interval: 10000ms
	//setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&tv), sizeof(int));

	// Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	// step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "ERROR " << WSAGetLastError() << ": Cannot connect server!!!" << endl;
		return -1;
	}

	// Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int res, messLen;

	while (true) {
		// Send mess
		cout << "Send to server: ";
		gets_s(buff, BUFF_SIZE);
		messLen = strlen(buff);
		if (!_stricmp(buff, SHUT_DOWN)) 
			break;

		res = send(client, buff, messLen, 0);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot send mess!!!" << endl;

		// Receive mess
		res = recv(client, buff, BUFF_SIZE, 0);
		if (res == SOCKET_ERROR) {
			/*if (WSAGetLastError() == WSAETIMEDOUT)
				cout << "Time-out!" << endl;*/
			//else
				cout << "Error " << WSAGetLastError() << ": Cannot receive mess!!!" << endl;
		}
		else if (res > 0) {
			buff[res] = 0;
			cout << "Receive from server: ";
			recvDataFromServer(buff);
		}
	}

	// Step 5: Close socket
	closesocket(client);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

