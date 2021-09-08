// SingleOEBServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

#define SERVER_ADDR "127.0.0.1"
#define PORT 5500
#define DATA_BUFSIZE 8192
#define RECEIVE 0
#define SEND 1

int main(int argc, char* argv[])
{
	SOCKET connSocket, listenSocket;
	WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
	DWORD flags = 0, nEvents = 0, recvBytes = 0, transferredBytes = 0, sentBytes = 0;
	int operation;
	WSAOVERLAPPED acceptOverlapped;
	WSABUF dataBuf;
	char buffer[DATA_BUFSIZE];

	WSADATA wsaData;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	// Step 1: Start Winsock and set up a listening socket
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported\n");
		return 0;
	}

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 1;
	}

	if (listen(listenSocket, 5)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 1;
	}

	printf("Server start!");

	//Setup the WSAEVENT project for listening socket
	if ((events[0] = WSACreateEvent()) == WSA_INVALID_EVENT) {
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}
	nEvents++;

	// Step 2: Accept an inbound connection
	if ((connSocket = accept(listenSocket, (PSOCKADDR)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
		printf("accept() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Step 3: Set up an overlapped structure
	events[nEvents] = WSACreateEvent();
	ZeroMemory(&acceptOverlapped, sizeof(WSAOVERLAPPED));
	acceptOverlapped.hEvent = events[nEvents];
	nEvents++;

	// Step 4: Post a WSARecv request to begin receiving data on the socket
	operation = RECEIVE;
	dataBuf.len = DATA_BUFSIZE;
	dataBuf.buf = buffer;

	if (WSARecv(connSocket, &dataBuf, 1, &transferredBytes, &flags, &acceptOverlapped, NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
			return 1;
		}
	}
	// Signal the first event in the event array to tell that a new client connected
	if (WSASetEvent(events[0]) == FALSE) {
		printf("WSASetEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	while (1) {
		DWORD index;
		// Step 5: Wait for the overlapped I/O call to complete
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		index = index - WSA_WAIT_EVENT_0;

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.
		if (index == 0) {
			WSAResetEvent(events[0]);
			continue;
		}

		// Step 6: Determine the status of the overlapped request
		WSAGetOverlappedResult(connSocket, &acceptOverlapped, &transferredBytes, FALSE, &flags);

		// Step 7: Reset the signaled event		
		WSAResetEvent(events[index]);

		// First check to see whether the peer has closed
		// the connection, and if so, close the socket
		if (transferredBytes == 0) {
			printf("Closing socket %d\n", connSocket);
			closesocket(connSocket);
			WSACloseEvent(events[index]);
			return 1;
		}

		// Check to see if the operation field equals RECEIVE. If this is so, then
		// this means a WSARecv call just completed
		if (operation == RECEIVE) {
			recvBytes = transferredBytes;	//the number of bytes which is received from client
			sentBytes = 0;					//the number of bytes which is sent to client
			operation = SEND;				//set operation to send reply message
		}
		else
			sentBytes += transferredBytes;

		// Step 8: Post another I/O operation
		// Since WSASend() is not guaranteed to send all of the bytes requested,
		// continue posting WSASend() calls until all received bytes are sent.
		if (recvBytes > sentBytes) {
			dataBuf.buf = buffer + sentBytes;
			dataBuf.len = recvBytes - sentBytes;
			operation = SEND;
			if (WSASend(connSocket, &dataBuf, 1, &transferredBytes, flags, &acceptOverlapped, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSASend() failed with error %d\n", WSAGetLastError());
					closesocket(connSocket);
					return 1;
				}
			}

		}
		else {
			// No more bytes to send post another WSARecv() request
			ZeroMemory(&acceptOverlapped, sizeof(WSAOVERLAPPED));
			acceptOverlapped.hEvent = events[index];
			recvBytes = 0;
			operation = RECEIVE;
			dataBuf.len = DATA_BUFSIZE;
			dataBuf.buf = buffer;
			flags = 0;

			if (WSARecv(connSocket, &dataBuf, 1, &transferredBytes, &flags, &acceptOverlapped, NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() != WSA_IO_PENDING) {
					printf("WSARecv() failed with error %d\n", WSAGetLastError());
					closesocket(connSocket);
					return 1;
				}
			}
		}
	}

	closesocket(listenSocket);
	WSACleanup();
	return 0;
}