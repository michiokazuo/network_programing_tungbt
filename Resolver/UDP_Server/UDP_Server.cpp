// UDP_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

int main()
{
	// Step 1: Init Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		cout << "Winsock 2.2 is not supported!!!" << endl;
		return -1;
	}

	// Step 2: Construct socket
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET) {
		cout << "Error "<< WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "Error " << WSAGetLastError() << ": Cannot bind this address!!!" << endl;
		return -1;
	}

	cout << "Server started!" << endl;

	// Step 4: Communicate with server
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int res, clientAddrLen = sizeof(clientAddr), clientPort;

	while (true) {
		// Receive mess
		res = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
		else if (res > 0) {
			buff[res] = 0;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Receive from client[" << clientIP << ":" << clientPort << "]: " << buff << endl;

			// Send to client
			res = sendto(server, buff, strlen(buff), 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
			if(res == SOCKET_ERROR)
				cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;
		}
	}

	// Step 5: Close socket
	closesocket(server);

	// Step 6: Terminate Winsock
	WSACleanup();

    return 0;
}

