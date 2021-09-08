// UDP_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#define SHUT_DOWN "bye"

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
	SOCKET client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (client == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	cout << "Client started!" << endl;

	// (optional) Set time-out for receiving
	int tv = 10000; // Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&tv), sizeof(int));

	// Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	// Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int res, serverAddrLen = sizeof(serverAddr), messLen;
	int sum_send_bytes = 0;

	while (true) {
		// Send mess
		cout << "Send to server: ";
		gets_s(buff, BUFF_SIZE);
		messLen = strlen(buff);
		if (!strcmp(buff, SHUT_DOWN)) {
			cout << "Sum bytes sended server: " << sum_send_bytes << endl;
			break;
		}

		res = sendto(client, buff, messLen, 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot send mess!!!" << endl;
		sum_send_bytes += res;

		// Receive mess
		res = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (res == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				cout << "Time-out!" << endl;
			else
				cout << "Error " << WSAGetLastError() << ": Cannot receive mess!!!" << endl;
		}
		else if (res > 0) {
			buff[res] = 0;
			cout << "Receive from server: " << buff << endl;
		}
	}

	// Step 5: Close socket
	closesocket(client);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

