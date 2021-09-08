// TCP_Server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

/**
* @function resolveDataFromClient: split data string from client to number string and alphabet string
*
* @param data: the string(valid data is string only contains alphabet and number) receive from client
*
* @return: a string result send to client after processing(if data is invalid, return Error)
**/
string resolveDataFromClient(string data) {
	string rs = "-";
	string numRS = "";
	string charRS = "";
	bool validData = true;
	int data_size = data.size();

	for (int i = 0; i < data_size; i++) {
		if ((data[i] >= 'a' && data[i] <= 'z') || (data[i] >= 'A' && data[i] <= 'Z'))
			charRS.push_back(data[i]);
		else if (data[i] >= '0' && data[i] <= '9')
			numRS.push_back(data[i]);
		else {
			validData = false;
			break;
		}
	}

	if (validData) {
		rs = "+ ";
		rs.append(charRS).append(" ").append(numRS);
	}

	return rs;
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
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	// Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[1]));
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "Error " << WSAGetLastError() << ": Cannot associate a local address with server socket!!!" << endl;
		return -1;
	}

	// Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		cout << "Error " << WSAGetLastError() << ": Cannot place server socket in state LISTEN!!!" << endl;
		return -1;
	}

	cout << "Server started!" << endl;

	// Step 5: Communicate with server
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int res, clientAddrLen = sizeof(clientAddr), clientPort;

	// accept request
	while (true) {

		SOCKET connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen);
		if (connSock == SOCKET_ERROR) {
			cout << "ERROR " << WSAGetLastError() << ": Cannot permit incoming connection!!!" << endl;
			return -1;
		}
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Accept incoming connection from " << clientIP << ":" << clientPort << endl;
		}

		while (true) {
			// Receive mess
			res = recv(connSock, buff, BUFF_SIZE, 0);
			if (res == SOCKET_ERROR) {
				cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
				break;
			}
			else if (res > 0) {
				buff[res] = 0;
				cout << "Receive from client[" << clientIP << ":" << clientPort << "]: " << buff << endl;

				// Send to client
				string rs = resolveDataFromClient(buff);

				res = send(connSock, rs.c_str(), rs.size(), 0);
				if (res == SOCKET_ERROR) {
					cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;
					break;
				}
			}
			else {
				cout << "Client Disconnects." << endl;
				break;
			}
		} // end communicating
		closesocket(connSock);
	}

	// Step 5: Close socket
	
	closesocket(listenSock);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

