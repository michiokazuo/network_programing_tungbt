//Server.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include "control.h"

CLIENT		clientInSystem[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];

void closeClientAndEvent(int, int);
void controllerServer(CLIENT *, int, DWORD &);

/**
* @function controllerServer: control system in server
*
* @param1 client: client send request to server
* @param2 index: the index of client has event in array
* @param3 nEvents: number of clients connect with server still active
*
**/
void controllerServer(CLIENT *client, int index, DWORD &nEvents) {

	if (!client->free) {
		int ret = 0;
		SOCKET connSock = client->socket;
		int mode = client->mode;

		if (mode == RECEIVE_FILE) {
			int opcode = 0, length = 0;
			char payload[PAYLOAD_BLOCK_SIZE];

			ret = RecvPacket(connSock, opcode, length, payload, PAYLOAD_BLOCK_SIZE);

			//Release socket and event if an error occurs
			if (ret <= 0) {
				closeClientAndEvent(index, nEvents);
				nEvents--;
			}
			else {
				bool finish;
				ret = RecvFile(connSock, client->currentPath, opcode, length, payload, finish);
				if (finish) {
					client->mode = NORMAL_TASK;
					goBack(client->currentPath, client->rootPath);
				}
				WSAResetEvent(events[index]);
			}
		}
		else if (mode == NORMAL_TASK) {
			Message msg_recv, msg_send;

			ret = recvMess(connSock, msg_recv);

			if (ret <= 0) {
				//Release socket and event if an error occurs
				closeClientAndEvent(index, nEvents);
				nEvents--;
			}
			else {
				cout << "Receive from client [" << client->clientIP << ":" << client->clientPort
					<< "]: " << (client->isLogin ? client->username : "Not Login")
					<< " $ " << msg_recv.opcode << endl;
				bool check_download = false;

				int rs = controllMessagse(clientInSystem, client, msg_recv, msg_send, check_download);

				ret = sendMess(connSock, msg_send);
				if (ret == SOCKET_ERROR)
					cout << "Error " << WSAGetLastError()
					<< ": Cannot send data to client[" << client->clientIP << ":" << client->clientPort << "] !!!" << endl;
				else if (check_download) {
					ret = SendFile(connSock, client->currentPath);
					goBack(client->currentPath, client->rootPath);
				}

				logManager(&clientInSystem[index], msg_recv, rs);

				//reset event
				//WSAResetEvent(events[index]);
			}
		}
		else {
			cout << "Mode is not define !!!!" << endl;
		}
	}
	else {
		cout << "Client is not connect!!!" << endl;
	}
}


/**
* @function closeClientAndEvent: close client and event when client close the connection or has error
*
* @param1 index: the index of client has error in array
* @param2 last: the last index of array has event
*
**/
void closeClientAndEvent(int index, int last) {
	closesocket(clientInSystem[index].socket);
	WSACloseEvent(events[last]);

	clientInSystem[index] = clientInSystem[last - 1];
	clientInSystem[last - 1] = CLIENT(true, NULL, -1, NULL, false, "", "", "", NORMAL_TASK);

	events[index] = WSACreateEvent();
	WSAEventSelect(clientInSystem[index].socket, events[index], FD_READ | FD_CLOSE);
}


int main(int argc, char* argv[])
{
	DWORD		nEvents = 0;
	DWORD		index;
	WSANETWORKEVENTS sockEvent;

	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		cout << "Winsock 2.2 is not supported!!!" << endl;
		return -1;
	}

	//Step 2: Construct LISTEN socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

	clientInSystem[0] = CLIENT(false, listenSock, SERVER_PORT, SERVER_ADDR, true, "", "", "", NORMAL_TASK);
	events[0] = WSACreateEvent(); //create new events
	nEvents++;

	// Associate event types FD_ACCEPT and FD_CLOSE
	// with the listening socket and newEvent   
	WSAEventSelect(clientInSystem[0].socket, events[0], FD_ACCEPT | FD_CLOSE);


	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		cout << "Error" << WSAGetLastError() << ": Cannot associate a local address with server socket." << endl;
		return -1;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		cout << "Error" << WSAGetLastError() << ": Cannot place server socket in state LISTEN." << endl;
		return -1;
	}

	cout << "SERVER STARTED WITH SERVER_PORT: " << SERVER_PORT << " AND SERVER_ADDRESS: " << SERVER_ADDR << " !" << endl << endl;
	cout << "INIT SYSTEM..." << endl;
	//create folder for containing clients' folders
	if (!fs::exists(ROOT_FOLDER)) {
		fs::create_directories(ROOT_FOLDER);
		cout << "Client Folders already exists!!!" << endl;
	}
	else {
		cout << "Create new Client Folders!!!" << endl;
	}
	createDB();
	cout << "DONE !!!" << endl << endl;

	SOCKET connSock;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;
	int i;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		clientInSystem[i] = { true, NULL, -1, NULL, false, "", "", "", NORMAL_TASK };
	}

	while (true) {
		//wait for network events on all socket
		index = WSAWaitForMultipleEvents(nEvents, events, FALSE, WSA_INFINITE, FALSE);
		if (index == WSA_WAIT_FAILED) {
			cout << "Error " << WSAGetLastError() << ": WSAWaitForMultipleEvents() failed!!!" << endl;
			break;
		}

		index = index - WSA_WAIT_EVENT_0;
		WSAEnumNetworkEvents(clientInSystem[index].socket, events[index], &sockEvent);

		if (sockEvent.lNetworkEvents & FD_ACCEPT) {
			if (sockEvent.iErrorCode[FD_ACCEPT_BIT] != 0) {
				cout << "FD_ACCEPT failed with error " << sockEvent.iErrorCode[FD_READ_BIT] << " !!!" << endl;
				continue;
			}

			if ((connSock = accept(clientInSystem[index].socket, (sockaddr *)&clientAddr, &clientAddrLen)) == SOCKET_ERROR) {
				cout << "Error " << WSAGetLastError() << ": Cannot permit incoming connection." << endl;
				continue;
			}

			//Add new socket into socks array
			int i;
			if (nEvents == WSA_MAXIMUM_WAIT_EVENTS) {
				cout << endl << "Error! Too many clients!!!" << endl << endl;
				closesocket(connSock);
			}
			else
				for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++)
					if (clientInSystem[i].free) {
						inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
						clientPort = ntohs(clientAddr.sin_port);

						cout << "You got a connection from " << clientIP << ":" << clientPort << endl; /* prints client's IP */

						clientInSystem[i] = { false, connSock, clientPort, clientIP, false, "", "", "", NORMAL_TASK };

						events[i] = WSACreateEvent();
						WSAEventSelect(clientInSystem[i].socket, events[i], FD_READ | FD_CLOSE);
						nEvents++;
						break;
					}

			//reset event
			WSAResetEvent(events[index]);
			continue;
		}

		if (sockEvent.lNetworkEvents & FD_READ) {
			//Receive message from client
			if (sockEvent.iErrorCode[FD_READ_BIT] != 0) {
				cout << "FD_READ failed with error " << sockEvent.iErrorCode[FD_READ_BIT] << " !!!" << endl;
				break;
			}

			controllerServer(&clientInSystem[index], index, nEvents);
			continue;
		}

		if (sockEvent.lNetworkEvents & FD_CLOSE) {
			if (sockEvent.iErrorCode[FD_CLOSE_BIT] != 0) {
				cout << "FD_CLOSE failed with error " << sockEvent.iErrorCode[FD_CLOSE_BIT] << " !!!" << endl;
			}

			//Release socket and event
			closeClientAndEvent(index, nEvents);
			nEvents--;
		}
	}

	return 0;
}