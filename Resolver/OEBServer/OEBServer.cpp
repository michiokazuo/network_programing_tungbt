// OEBServer.cpp : This sample illustrates how to develop a simple echo server Winsock
// application using the Overlapped I/O model with event notification.
//

#include "stdafx.h"
#include "winsock2.h"
#include "stdio.h"
#include "WS2tcpip.h"
#include "string.h"
#define SERVER_ADDR "127.0.0.1"
#define PORT 5500
#define DATA_BUFSIZE 8192
#define RECEIVE 0
#define SEND 1
#pragma comment(lib, "Ws2_32.lib")

/*Struct contains information of the socket communicating with client*/
typedef struct SocketInfo {
	SOCKET socket;
	WSAOVERLAPPED overlapped;
	WSABUF dataBuf;
	char buffer[DATA_BUFSIZE];
	int operation;
	int sentBytes;
	int recvBytes;
};

void freeSockInfo(SocketInfo* siArray[], int n);
void closeEventInArray(WSAEVENT eventArr[], int n);

int main()
{
	SocketInfo* socks[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT events[WSA_MAXIMUM_WAIT_EVENTS];
	int nEvents = 0;

	WSADATA wsaData;
	if (WSAStartup((2, 2), &wsaData) != 0) {
		printf("WSAStartup() failed with error %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	for (int i = 0; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		socks[i] = 0;
		events[i] = 0;
	}

	// Start Winsock and set up a LISTEN socket
	SOCKET listenSocket;
	if ((listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET) {
		printf("Error %d: Cannot create server socket.", WSAGetLastError());
		return 1;
	}

	// LISTEN socket associate with an event object 
	events[0] = WSACreateEvent();
	nEvents++;
	WSAEventSelect(listenSocket, events[0], FD_ACCEPT | FD_CLOSE);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	if (bind(listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error %d: Cannot associate a local address with server socket.", WSAGetLastError());
		return 0;
	}

	if (listen(listenSocket, 10)) {
		printf("Error %d: Cannot place server socket in state LISTEN.", WSAGetLastError());
		return 0;
	}

	printf("Server started!\n");

	int index, ret;
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (1) {
		// Wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			printf("Error %d: WSAWaitForMultipleEvents() failed\n", WSAGetLastError());
			return 0;
		}

		index = index - WSA_WAIT_EVENT_0;
		DWORD flags, transferredBytes;

		// If the event triggered was zero then a connection attempt was made
		// on our listening socket.
		if (index == 0) {
			WSAResetEvent(events[0]);
			if ((connSock = accept(listenSocket, (sockaddr *)&clientAddr, &clientAddrLen)) == INVALID_SOCKET) {
				printf("Error %d: Cannot permit incoming connection.\n", WSAGetLastError());
				return 0;
			}

			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				printf("\nToo many clients.");
				closesocket(connSock);
			}
			else {
				// Disassociate connected socket with any event object
				WSAEventSelect(connSock, NULL, 0);

				// Append connected socket to the array of SocketInfo
				i = nEvents;
				events[i] = WSACreateEvent();
				socks[i] = (SocketInfo *)malloc(sizeof(SocketInfo));
				socks[i]->socket = connSock;
				memset(&socks[i]->overlapped, 0, sizeof(WSAOVERLAPPED));
				socks[i]->overlapped.hEvent = events[i];
				socks[i]->dataBuf.buf = socks[i]->buffer;
				socks[i]->dataBuf.len = DATA_BUFSIZE;
				socks[i]->operation = RECEIVE;
				socks[i]->recvBytes = 0;
				socks[i]->sentBytes = 0;

				nEvents++;

				// Post an overlpped I/O request to begin receiving data on the socket
				flags = 0;
				if (WSARecv(socks[i]->socket, &(socks[i]->dataBuf), 1, &transferredBytes, &flags, &(socks[i]->overlapped), NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						printf("WSARecv() failed with error %d\n", WSAGetLastError());
						closeEventInArray(events, i);
						freeSockInfo(socks, i);
						nEvents--;
					}
				}

			}
		}
		else { // If the event triggered wasn't zero then an I/O request is completed.
			SocketInfo *client;
			client = socks[index];
			WSAResetEvent(events[index]);
			BOOL result;
			result = WSAGetOverlappedResult(client->socket, &(client->overlapped), &transferredBytes, FALSE, &flags);
			if (result == FALSE || transferredBytes == 0) {
				closesocket(client->socket);
				closeEventInArray(events, index);
				freeSockInfo(socks, index);
				client = 0;
				nEvents--;
				continue;
			}

			// Check to see if the operation field equals RECEIVE. If this is so, then
			// this means a WSARecv call just completed
			if (client->operation == RECEIVE) {
				client->recvBytes = transferredBytes;	//the number of bytes which is received from client
				client->sentBytes = 0;					//the number of bytes which is sent to client
				client->operation = SEND;				//set operation to send reply message
			}
			else
				client->sentBytes += transferredBytes;

			// Post another I/O operation
			// Since WSASend() is not guaranteed to send all of the bytes requested,
			// continue posting WSASend() calls until all received bytes are sent.
			if (client->recvBytes > client->sentBytes) {
				client->dataBuf.buf = client->buffer + client->sentBytes;
				client->dataBuf.len = client->recvBytes - client->sentBytes;
				client->operation = SEND;
				if (WSASend(client->socket, &(client->dataBuf), 1, &transferredBytes, flags, &(client->overlapped), NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						printf("WSASend() failed with error %d\n", WSAGetLastError());
						closesocket(client->socket);
						closeEventInArray(events, index);
						freeSockInfo(socks, index);
						client = 0;
						nEvents--;
						continue;
					}
				}

			}
			else {
				// No more bytes to send post another WSARecv() request
				memset(&(client->overlapped), 0, sizeof(WSAOVERLAPPED));
				client->overlapped.hEvent = events[index];
				client->recvBytes = 0;
				client->operation = RECEIVE;
				client->dataBuf.buf = client->buffer;
				client->dataBuf.len = DATA_BUFSIZE;
				flags = 0;

				if (WSARecv(client->socket, &(client->dataBuf), 1, &transferredBytes, &flags, &(client->overlapped), NULL) == SOCKET_ERROR) {
					if (WSAGetLastError() != WSA_IO_PENDING) {
						printf("WSARecv() failed with error %d\n", WSAGetLastError());
						closesocket(client->socket);
						closeEventInArray(events, index);
						freeSockInfo(socks, index);
						client = 0;
						nEvents--;
					}
				}
			}
		}
	}

}

/**
* The freeSockInfo function remove a socket from array
* @param	siArray		An array of pointers of socket information struct
* @param	n	Index of the removed socket
*/

void freeSockInfo(SocketInfo* siArray[], int n) {
	closesocket(siArray[n]->socket);
	free(siArray[n]);
	siArray[n] = 0;
	for (int i = n; i < WSA_MAXIMUM_WAIT_EVENTS - 1; i++) {
		siArray[i] = siArray[i + 1];
	}
}

/**
* The closeEventInArray function release an event and remove it from an array
* @param	eventArr	An array of event object handles
* @param	n	Index of the removed event object
*/
void closeEventInArray(WSAEVENT eventArr[], int n) {
	WSACloseEvent(eventArr[n]);

	for (int i = n; i < WSA_MAXIMUM_WAIT_EVENTS - 1; i++)
		eventArr[i] = eventArr[i + 1];
}