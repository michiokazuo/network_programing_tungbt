#include "stdafx.h"
#include "TransferData.h"
#include "View.h"

using namespace std;
#define PORT 6000
#define SERVER_ADDR "127.0.0.1"


int main(int argc, char* argv[]) {
	// Step 1: Initiate WinSock
	WSAData wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("WinSock 2.2 is not supported\n");
		return 0;
	}

	// Step 2: Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: Cannot create TCP socket! ", WSAGetLastError());
		return 0;
	}

	// Step 3: Specify server address
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	// Step 4: Request to connect server
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect to server", WSAGetLastError());
		return 0;
	}
	printf("Connected to server!\n");

	// Step 5: Communicate with server and Start the service
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	startService(client, hConsole);


	// Step 6: Close socket
	closesocket(client);

	// Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}
