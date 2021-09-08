#include "stdafx.h"
#define BUFF_SIZE 2048

/**
* @function split: split a string into an array of data based on the delimiters
*
* @param data: the char delimeter used to split the string
*
* @return: vecto<string> after split
**/
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
* @function isValidIP: check data receive from input is IP or not(Domain)
*
* @param data: the string receive from input
*
* @return: true or false
**/
bool isValidIP(char *data) {
	in_addr addr;
	return inet_pton(AF_INET, data, &addr) == 1;
}

/**
* @function recvDataFromServer: print data receive from server to console
*
* @param1 data: the string receive from server
* @param2 isIP: specify type data receive from server is IP or Domain
*
**/
void recvDataFromServer(char *data, bool isIP) {
	if (data[0] == '-')
		cout << "Not found information" << endl;
	else if (data[0] == '+') {
		vector<string> rs = split(data, ' ');
		int rs_size = rs.size();
		if (rs_size > 1) {
			string type = isIP ? "name" : "IP";

			for (int i = 1; i < rs_size; i++) {
				if (i == 1)
					cout << "Official " << type << ": ";
				if (i == 2)
					cout << "Alias " << type << ": ";
				cout << rs[i] << endl;
			}
		}
		else 
			cout << "Not found information" << endl;
	}
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
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	// Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int res, serverAddrLen = sizeof(serverAddr), messLen;

	while (true) {
		// Send request to server
		cout << "Send to server: ";
		gets_s(buff, BUFF_SIZE);
		messLen = strlen(buff);
		if (!messLen) // when input is '', exit program
			break;

		bool isIP = isValidIP(buff);

		res = sendto(client, buff, messLen, 0, (sockaddr *)&serverAddr, serverAddrLen);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot send mess!!!" << endl;

		// Receive result from server
		res = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);
		if (res == SOCKET_ERROR) {
			if (WSAGetLastError() == WSAETIMEDOUT)
				cout << "Time-out!!!" << endl;
			else
				cout << "Error " << WSAGetLastError() << ": Cannot receive mess!!!" << endl;
		}
		else if (res > 0) {
			buff[res] = 0;
			// Processing result from server
			cout << "Receive from server: "; 
			recvDataFromServer(buff, isIP);
		}
	}

	// Step 5: Close socket
	closesocket(client);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

