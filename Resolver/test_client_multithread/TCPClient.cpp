
// TCPClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "process.h"
#define SERVER_PORT 6969
#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048
#pragma comment (lib, "Ws2_32.lib")
int serverPort;
unsigned __stdcall thread1(void *param);
unsigned __stdcall thread2(void *param);

int main(int argc, char* argv[])
{
	//Step 1: Inittiate WinSock
	WSADATA wsaData;

	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Specify server address

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server. %d", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Connected server!\n");
	int tv = 100; //Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));
	//Step 5: Communicate with server
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];
	int ret;

	strcpy(sBuff, "POST Hello\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Sequence test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "USER ductq\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n\n", sBuff, rBuff);

	strcpy(sBuff, "USER admin\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "POST Hello\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "QUIT \r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "foo\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Syntax test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "USER admin\r\nPOST Hello world\r\nPOST Test stream\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	if (ret < 0)
		printf("Stream test fail!\n");
	else {
		rBuff[ret] = 0;
		printf("Main: %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "POST I am tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Main: %s-->%s\n", sBuff, rBuff);
	_getch();

	_beginthreadex(0, 0, thread1, 0, 0, 0); //start thread
	_beginthreadex(0, 0, thread2, 0, 0, 0); //start thread

	Sleep(2000);

	int numConn = 0;
	printf("Concurent connections: ");
	scanf_s("%d", &numConn);
	if (numConn > 0) {

		char buff[BUFF_SIZE];
		int numSession = 0, numConnected = 0;
		SOCKET clients[4096];
		for (int i = 0; i < numConn; i++) {
			clients[i] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (connect(clients[i], (sockaddr *)&serverAddr, sizeof(serverAddr))) {
				printf("\nError: %d", WSAGetLastError());
				break;
			}
			numConnected++;
			int tv2 = 20;
			setsockopt(clients[i], SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv2), sizeof(int));

			strcpy(sBuff, "USER admin\r\n");
			ret = send(clients[i], sBuff, strlen(sBuff), 0);
			ret = recv(clients[i], rBuff, BUFF_SIZE, 0);

			if (ret < 0)
				printf("recv() fail.\n");
			else {
				rBuff[ret] = 0;
				printf("Concurent test: %s\n", rBuff);
				numSession++;
			}
		}

		printf("\nNumber of success connection: %d", numConnected);
		printf("\nNumber of success session: %d\n", numSession);

		for (int i = 0; i < numConn; i++)
			closesocket(clients[i]);
	}

	//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}

unsigned __stdcall thread1(void *param)
{
	//Step 2: Construct socket	
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server. %d", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Connected server!\n");

	//Step 5: Communicate with server
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];
	int ret;
	strcpy(sBuff, "USER tungbt\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 1:  %s-->%s\n", sBuff, rBuff);

	for (int i = 0; i < 5; i++) {
		Sleep(100);
		strcpy(sBuff, "POST Hello. I am tungbt\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);
		rBuff[ret] = 0;
		printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "QUIT \r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 1:  %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "USER test\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 1:  %s-->%s\n", sBuff, rBuff);

	for (int i = 0; i < 5; i++) {
		Sleep(100);
		strcpy(sBuff, "POST Hello. I am test\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);
		rBuff[ret] = 0;
		printf("Thread 1:  %s-->%s\n", sBuff, rBuff);
	}

	//Step 6: Close socket
	closesocket(client);
	printf("Thread 1 end.\n");
}

unsigned __stdcall thread2(void *param)
{
	Sleep(10);
	//Step 2: Construct socket	
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server. %d", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Connected server!\n");

	//Step 5: Communicate with server
	char sBuff[BUFF_SIZE], rBuff[BUFF_SIZE];
	int ret;
	strcpy(sBuff, "USER admin\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
	for (int i = 0; i < 10; i++) {
		Sleep(100);
		strcpy(sBuff, "POST Hello. I am admin\r\n");
		ret = send(client, sBuff, strlen(sBuff), 0);
		ret = recv(client, rBuff, BUFF_SIZE, 0);
		rBuff[ret] = 0;
		printf("Thread 2:  %s-->%s\n", sBuff, rBuff);
	}

	strcpy(sBuff, "QUIT \r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 1:  %s-->%s\n", sBuff, rBuff);

	strcpy(sBuff, "USER ductq\r\n");
	ret = send(client, sBuff, strlen(sBuff), 0);
	ret = recv(client, rBuff, BUFF_SIZE, 0);
	rBuff[ret] = 0;
	printf("Thread 2:  %s-->%s\n", sBuff, rBuff);

	//Step 6: Close socket
	closesocket(client);
	printf("Thread 2 end.\n");
}