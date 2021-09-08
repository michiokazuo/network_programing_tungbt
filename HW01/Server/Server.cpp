#include "stdafx.h"
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

/**
* @function isValidIP: check data receive from client is IP or not(Domain) 
* 
* @param data: the string receive from client
*
* @return: true or false
**/
bool isValidIP(char *data) {
	in_addr addr;
	return inet_pton(AF_INET, data, &addr) == 1;
}

/**
* @function getDomainFromIP: get all domain(official, alias) from IP address
*
* @param IP: the string(IP) receive from client
*
* @return: a string result send to client after processing
**/
string getDomainFromIP(char* IP) {
	string rs = "";
	struct hostent* host_info;
	struct in_addr addr;

	inet_pton(AF_INET, IP, &addr);
	host_info = gethostbyaddr((char*)&addr, 4, AF_INET);

	if (host_info != NULL) {
		rs = "+ ";
		rs += host_info->h_name;

		for (int i = 0; host_info->h_aliases[i] != 0; i++) {
			rs += ' ';
			rs += host_info->h_aliases[i];
		}
	}
	else {
		rs = "-";
		cout << "gethostbyaddr() error: " << WSAGetLastError() << endl;
	}

	return rs;
}

/**
* @function getIPFromDomain: get all IP(official, alias) from domain
*
* @param domain: the string(domain) receive from client
*
* @return: a string result send to client after processing
**/
string getIPFromDomain(char * domain) {
	string rs = "";
	addrinfo *result; //pointer to the linked-list 
					  //containing information about the host
	int rc;
	sockaddr_in *address;
	addrinfo hints; //hint structure
	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET; //only focus on IPv4 address
	rc = getaddrinfo(domain, NULL, &hints, &result);

	//Get the address info
	char ipStr[INET_ADDRSTRLEN];

	if (rc == 0) {
		rs = "+";
		while (result != NULL) {
			address = (struct sockaddr_in *) result->ai_addr;
			inet_ntop(AF_INET, &address->sin_addr, ipStr, sizeof(ipStr));
			rs += ' ';
			rs += ipStr;
			result = result->ai_next;
		}
	}
	else {
		rs = "-";
		cout << "getaddrinfo() error: " << WSAGetLastError() << endl;
	}

	freeaddrinfo(result);
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
	SOCKET server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	// Step 3: Bind address to socket
	const int SERVER_PORT = atoi(argv[1]);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(server, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "Error " << WSAGetLastError() << ": Cannot bind this address!!!" << endl;
		return -1;
	}

	cout << "SERVER STARTED WITH SERVER_PORT: "<< SERVER_PORT << " AND SERVER_ADDRESS: " << SERVER_ADDR << "!" << endl;

	// Step 4: Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
	int res, clientAddrLen = sizeof(clientAddr), clientPort;

	while (true) {
		// Receive request from client
		res = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr *)&clientAddr, &clientAddrLen);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
		else if (res > 0) {
			buff[res] = 0;
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Receive from client[" << clientIP << ":" << clientPort << "]: " << buff << endl;

			// Processing request from client
			string rs = "";

			if (isValidIP(buff))
				rs = getDomainFromIP(buff);
			else
				rs = getIPFromDomain(buff);

			// Echo to client
			res = sendto(server, rs.c_str(), rs.size(), 0, (sockaddr *)&clientAddr, sizeof(clientAddr));
			if (res == SOCKET_ERROR)
				cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;
		}
	} // end while

	// Step 5: Close socket
	closesocket(server);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

