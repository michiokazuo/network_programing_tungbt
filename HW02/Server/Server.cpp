#include "stdafx.h"
#define SERVER_ADDR "127.0.0.1"

/**
* @function resolveDataFromClient: split data string from client to number string and alphabet string
*
* @param data: the string(valid data is string only contains alphabet and number) receive from client
*
* @return: valid data - a string result send to client after processing
		   invalid data - Error
**/
string resolveDataFromClient(string data) {
	string rs = "-";
	string numRS = "";
	string charRS = "";
	bool validData = true;
	int data_size = data.size();

	for (int i = 0; i < data_size; i++) {
		if ((data[i] >= 'a' && data[i] <= 'z') || (data[i] >= 'A' && data[i] <= 'Z')) // check for alphabet character
			charRS.push_back(data[i]);
		else if (data[i] >= '0' && data[i] <= '9') // check for numeric characters
			numRS.push_back(data[i]);
		else {
			// invalid data
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

/**
* @function customeSend: send a string(data size - 10bytes, data) to client specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to send data
* @param2 data: the string result after processing need to send client
*
* @return: successul - number of bytes sent
*		   failed - SOCKET_ERROR
**/
int customeSend(SOCKET &socket, string data) {
	int rs = 0;
	int size = data.size();
	string data_size;
	// append data size - 10bytes
	int num_size = (int)log10(size) + 1;
	data_size.append(to_string(size));
	for (int i = 1; i <= 10 - num_size; i++)
		data_size.append(" ");
	// append data 
	data_size.append(data);

	// send string(data size, size) to client to create buff_size
	rs = send(socket, data_size.c_str(), data_size.size(), 0);

	return rs;
}

/**
* @function customeRecv: receive a string(data size - 10bytes, data) from client to specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to receive data
* @param2 data: the string receive data from client
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int customeRecv(SOCKET &socket, string &data) {
	char buff[20];
	data = "";
	int rs = 0;
	// receive data size from client to create buff size
	rs = recv(socket, buff, 10, MSG_WAITALL);
	if (rs > 0) {
		buff[rs] = 0;
		int buff_size = atoi(buff);
		if (buff_size > 0) {
			char *tmp = new char[buff_size + 10];

			// receive data from client from receive data size
			rs = recv(socket, tmp, buff_size, 0);
			if (rs > 0) {
				tmp[rs] = 0;
				data.append(tmp);
			}
			delete[] tmp;
		}
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
	const int SERVER_PORT = atoi(argv[1]);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
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

	cout << "SERVER STARTED WITH SERVER_PORT: " << SERVER_PORT << " AND SERVER_ADDRESS: " << SERVER_ADDR << " !" << endl << endl;

	// Step 5: Communicate with server
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int res, clientAddrLen = sizeof(clientAddr), clientPort;
	string rs;

	while (true) {
		// Accept request from client
		SOCKET connSock = accept(listenSock, (sockaddr *)&clientAddr, &clientAddrLen);
		if (connSock == SOCKET_ERROR) {
			cout << "ERROR " << WSAGetLastError() << ": Cannot permit incoming connection!!!" << endl;
		}
		else {
			inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientAddr.sin_port);
			cout << "Accept incoming connection from " << clientIP << ":" << clientPort << endl;
		}

		while (true) {
			// Receive request from client
			res = customeRecv(connSock, rs);
			if (res == SOCKET_ERROR) {
				cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
				break;
			}
			else if (res > 0) {
				cout << "Receive from client[" << clientIP << ":" << clientPort << "]: " << rs << endl;

				// Processing request from client
				rs = resolveDataFromClient(rs);

				// Echo to client
				res = customeSend(connSock, rs);
				if (res == SOCKET_ERROR) {
					cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;
					break;
				}
			}
			else {
				cout << "Client " << clientIP << ":" << clientPort << " disconnected." << endl << endl;
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

