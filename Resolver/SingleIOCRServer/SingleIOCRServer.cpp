// SingleIOCRServer.cpp : Defines the entry point for the console application.
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

SOCKET acceptSocket;
WSABUF dataBuf;
char buffer[DATA_BUFSIZE];
int operation;

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED lpOverlapped, DWORD inFlags);

int main(int argc, char* argv[])
{
	DWORD flags, recvBytes, index;

	WSADATA wsaData;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrLen = sizeof(clientAddr);
	WSAEVENT events[1];
	SOCKET listenSocket;
	WSAOVERLAPPED overlapped;

	// Step 1: Start Winsock, and set up a listening socket
	if (WSAStartup((2, 2), &wsaData)) {
		printf("WSAStartup() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
	if (bind(listenSocket, (PSOCKADDR)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	if (listen(listenSocket, 5)) {
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	printf("Server started!");

	if ((events[0] = WSACreateEvent()) == WSA_INVALID_EVENT) {
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	// Step 2: Accept a new connection
	if ((acceptSocket = accept(listenSocket, (PSOCKADDR)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
		printf("accept() failed with error %d\n", WSAGetLastError());
		return 1;
	}

	//Step 3: Set up an overlapped structure
	ZeroMemory(&overlapped, sizeof(WSAOVERLAPPED));
	operation = RECEIVE;
	dataBuf.len = DATA_BUFSIZE;
	dataBuf.buf = buffer;
	flags = 0;

	//Step 4: Call I/O functions
	if (WSARecv(acceptSocket, &dataBuf, 1, &recvBytes, &flags, &overlapped, workerRoutine) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
			return 1;
		}
	}

	while (1) {
		index = WSAWaitForMultipleEvents(1, events, FALSE, WSA_INFINITE, TRUE);
		if (index == WSA_WAIT_FAILED) {
			printf("WSAWaitForMultipleEvents() failed with error %d\n", WSAGetLastError());
			return 1;
		}

		if (index != WAIT_IO_COMPLETION)
			break;
	}

	WSAResetEvent(events[index - WSA_WAIT_EVENT_0]);
	return 0;
}

void CALLBACK workerRoutine(DWORD error, DWORD transferredBytes, LPWSAOVERLAPPED lpOverlapped, DWORD inFlags) {

	DWORD sentBytes = 0, recvBytes = 0;
	DWORD flags = 0;
	if (error != 0)
		printf("I/O operation failed with error %d\n", error);

	if (transferredBytes == 0)
		printf("Connection closed\n");

	if (error != 0 || transferredBytes == 0) {
		// Either a bad error occurred on the socket or the socket was closed by a peer
		closesocket(acceptSocket);
		return;
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

	// Since WSASend() is not guaranteed to send all of the bytes requested,
	// continue posting WSASend() calls until all received bytes are sent.
	if (recvBytes > sentBytes) {
		dataBuf.buf = buffer + sentBytes;
		dataBuf.len = recvBytes - sentBytes;
		operation = SEND;

		if (WSASend(acceptSocket, &dataBuf, 1, &transferredBytes, flags, lpOverlapped, workerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSASend() failed with error %d\n", WSAGetLastError());
				closesocket(acceptSocket);
				return;
			}
		}

	}
	else {
		// No more bytes to send post another WSARecv() request
		ZeroMemory(lpOverlapped, sizeof(WSAOVERLAPPED));
		recvBytes = 0;
		operation = RECEIVE;
		dataBuf.len = DATA_BUFSIZE;
		dataBuf.buf = buffer;
		flags = 0;

		if (WSARecv(acceptSocket, &dataBuf, 1, &transferredBytes, &flags, lpOverlapped, workerRoutine) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING) {
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				closesocket(acceptSocket);
				return;
			}
		}
	}

}
