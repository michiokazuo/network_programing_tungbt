// WSAEventSelectServer.cpp : Defines the entry point for the console application.

#include "stdafx.h"

// contruct client in the system 
typedef struct {
	bool			isActive = false;	// client is connect or not
	SOCKET			socket = NULL;		// socket of client
	sockaddr_in		clientAddr;			// address of client
	string			username = "";		// username of account login on client
	bool			isLogin = false;	// client has account login or not
} Client;

Client		clientInSystem[WSA_MAXIMUM_WAIT_EVENTS];
WSAEVENT	events[WSA_MAXIMUM_WAIT_EVENTS];

/**
* @function split: split a string into an array of data based on the delimiters
*
* @param data: the string delimeter used to split the string
*
* @return: vecto<string> after split
**/
vector<string> split(string s, string delimiter) {
	vector<string> list;
	size_t pos = 0;
	string token;

	while ((pos = s.find(delimiter)) != string::npos) {
		token = s.substr(0, pos);
		list.push_back(token);
		s.erase(0, pos + delimiter.length());
	}

	list.push_back(s);
	return list;
}

/**
* @function getCurrentTimeInSystem: get current time in system
*
* @return: a string current time has been formatted
**/
const string getCurrentTimeInSystem() {
	time_t rawtime;
	struct tm timeinfo;
	char curr_time[80];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);
	strftime(curr_time, sizeof(curr_time), "[%d/%m/%Y %H:%M:%S]", &timeinfo);

	return curr_time;
}

/**
* @function intToBytes: convert int to a string has size to denpend on @param2
*
* @param1 num: the number int need to convert
* @param2 size: the size of string return
*
* @return: a string after converting number
**/
string intToBytes(int num, int size) {
	string rs;

	int num_size = (int)log10(abs(num)) + 1;
	if (num < 0) num_size++;
	rs.append(to_string(num));
	for (int i = 1; i <= size - num_size; i++)
		rs.append(" ");

	return rs;
}

/**
* @function customeSend: send a packet(transactionId - 10bytes, data size - 10bytes, data)
*						to client get transactionID to specify packet return client
*							and specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to send data
* @param2 tstID_req: transaction ID from packet receive from client
* @param3 data: the string receive from client
*
* @return: successul - number of bytes sent
*		   failed - SOCKET_ERROR
**/
int customSend(SOCKET &socket, int tstID_req, string data) {
	int rs = 0;
	string packet;
	// data append the character end of packet
	data.append(DELIMITER);

	// append transaction ID size - 10bytes
	packet.append(intToBytes(tstID_req, HEADER_SIZE));

	// append data size - 10bytes
	int data_size = data.size();
	packet.append(intToBytes(data_size, HEADER_SIZE));

	// append data
	packet.append(data);

	// send string(tstID , data_size, data) to server to marker packet and create buff_size
	int packet_size = packet.size();
	int index = 0;
	while (packet_size > 0) {
		rs = send(socket, packet.c_str() + index, packet_size, 0);
		if (rs == SOCKET_ERROR)
			break;
		packet_size -= rs;
		index += rs;
	}

	return rs == SOCKET_ERROR ? rs : index;
}

/**
* @function recvPartOfPacket: receive part of packet(transactionId, data size, data) from client
*
* @param1 socket: socket connect with client to receive data
* @param2 part: part of packet
* @param3 size: size of part
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int recvPartOfPacket(SOCKET &socket, string &part, int size) {
	char *buff = new char[size + PADDING_BUFF];
	int rs = 0;
	part = "";

	int ret;
	while (size > 0) {
		ret = recv(socket, buff, size, 0);

		if (ret == SOCKET_ERROR) {
			rs = rs > 0 ? rs : SOCKET_ERROR;
			break;
		}

		if (ret > 0) {
			buff[ret] = 0;
			rs += ret;
			part.append(buff);
			size -= ret;
			memset(buff, 0, sizeof(buff));
		}
	}

	delete[] buff;
	return rs;
}

/**
* @function customeRecv: receive a packet(transactionId, data size, data) from client
*						to get transactionID request and specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to receive data
* @param2 tstID_req: transaction ID from packet receive from client
* @param3 data: the string receive data from client
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int customRecv(SOCKET &socket, int &tstID_req, string &data) {
	int rs = 0;
	int bytes = 0;
	data = "";

	// receive transactionID from packet
	string tstID;
	rs = recvPartOfPacket(socket, tstID, HEADER_SIZE);

	if (rs > 0) {
		tstID_req = atoi(tstID.c_str());

		if (tstID_req <= 0) {
			tstID_req = -1;
			return CODE_PACKET_RECEIVE_ERROR;
		}

		bytes += rs;

		// receive data size from packet
		string size;
		rs = recvPartOfPacket(socket, size, HEADER_SIZE);

		if (rs > 0) {
			bytes += rs;
			int data_size = atoi(size.c_str());

			if (data_size > 0) {
				rs = recvPartOfPacket(socket, data, data_size);

				if (rs == SOCKET_ERROR)
					return rs;

				bytes += rs;
				int size_delim = DELIMITER.size();
				if (data.compare(data.size() - size_delim, size_delim, DELIMITER) != 0)
					rs = CODE_PACKET_RECEIVE_ERROR;

				data = data.substr(0, data.size() - size_delim);
			}
			else
				rs = CODE_PACKET_RECEIVE_ERROR;
		}
	}

	return rs == SOCKET_ERROR ? rs : bytes;
}

/**
* @function logManager: record request from client
*
* @param1 client: client send request to server
* @param2 data: the data of request from client
* @param3 rs: result from server send to client after processing request
*
**/
void logManager(Client *client, string data, int rs) {
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client->clientAddr.sin_addr, clientIP, sizeof(clientIP));
	int clientPort = ntohs(client->clientAddr.sin_port);

	stringstream log;
	log << clientIP << ":" << clientPort << " " << getCurrentTimeInSystem() << " $ " << data << " $ " << rs;

	ofstream logFile;
	logFile.open(LOG_20183966, ios::app);
	logFile << log.str() << endl;
	logFile.close();
}

/**
* @function checkAccountInSystem: check account exists in the system or not
*
* @param username: username of account
*
* @return: result code
**/
int checkAccountInSystem(string username) {
	ifstream account(ACCOUNT_FILE);
	string line;

	if (account.is_open()) {
		while (getline(account, line)) {
			vector<string> acc = split(line, " ");
			if (username.compare(acc[0]) == 0) {
				if (atoi(acc[1].c_str()) == 0)
					return 0;
				else if (atoi(acc[1].c_str()) == 1)
					return 1;
			}
		}
		return -1;
	}
	else {
		cout << "ERROR: " + ACCOUNT_FILE + " cannot open or not found!!!" << endl;
		return CODE_ERROR_SYSTEM_DEFAULT;
	}
}

/**
* @function processReqLogIn: process login request from client
*
* @param1 client: client send request to server
* @param2 req_0: the message code of request from client
* @param3 data: the data of packet receive from client
*
* @return: result code
**/
int processReqLogIn(Client *client, string req_0, string data) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (client->isLogin)
		return CODE_LOGIN_FAIL_ALREADY_SIGNED;

	string username = data.substr(req_0.size() + 1);

	int checkAcc = checkAccountInSystem(username);
	if (!checkAcc) {
		rs = CODE_LOGIN_SUCCESS;
		client->username = username;
		client->isLogin = true;
	}
	else {
		if (checkAcc != CODE_ERROR_SYSTEM_DEFAULT)
			rs = checkAcc > 0 ? CODE_LOGIN_FAIL_ACCOUNT_BLOCK : CODE_LOGIN_FAIL_NOT_EXIST_ACCOUNT;
	}

	return rs;
}

/**
* @function processReqPostMessage: process post message request from client
*
* @param client: client send request to server
*
* @return: result code
**/
int processReqPostMessage(Client *client) {
	return client->isLogin ? CODE_POST_SUCCESS : CODE_POST_FAIL_NOT_SINGED;
}

/**
* @function processReqLogOut: process post message request from client
*
* @param client: client send request to server
*
* @return: result code
**/
int processReqLogOut(Client *client) {
	if (!(client->isLogin))
		return CODE_LOGOUT_FAIL_NOT_SINGED;

	client->username = "";
	client->isLogin = false;

	return CODE_LOGOUT_SUCCESS;
}

/**
* @function controllerServer: control system in server
*
* @param client: client send request to server
* @param data: the data of packet receive from client
*
* @return: result code
**/
int controllerServer(Client *client, string data) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;
	if (client->isActive && data.size() > 0) {
		vector<string> req = split(data, " ");
		string func = req[0];

		if (req.size() > 1) {
			if (func.compare(FORMAT_LOGIN) == 0)
				rs = processReqLogIn(client, req[0], data);
			else if (func.compare(FORMAT_POST_MESS) == 0)
				rs = processReqPostMessage(client);
		}
		else {
			if (func.compare(FORMAT_LOGOUT) == 0)
				rs = processReqLogOut(client);
		}
	}

	return rs;
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
	clientInSystem[last - 1].isActive = false;
	clientInSystem[last - 1].socket = NULL;
	clientInSystem[last - 1].isLogin = false;
	clientInSystem[last - 1].username = "";
	memset(&clientInSystem[last - 1].clientAddr, 0, sizeof(clientInSystem[last - 1].clientAddr));

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
	inet_pton(AF_INET, SERVER_ADDR.c_str(), &serverAddr.sin_addr);

	clientInSystem[0].isActive = true;
	clientInSystem[0].isLogin = true;
	clientInSystem[0].username = "listenSock";
	clientInSystem[0].socket = listenSock;
	clientInSystem[0].clientAddr = serverAddr;
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

	SOCKET connSock;
	sockaddr_in clientAddr;
	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;
	int ret, i;
	string req;
	int tstID;

	for (i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		clientInSystem[i].isActive = false;
		clientInSystem[i].socket = NULL;
		clientInSystem[i].isLogin = false;
		clientInSystem[i].username = "";
		memset(&clientInSystem[i].clientAddr, 0, sizeof(clientInSystem[i].clientAddr));
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
					if (!clientInSystem[i].isActive) {
						inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
						clientPort = ntohs(clientAddr.sin_port);

						cout << "You got a connection from " << clientIP << ":" << clientPort << endl; /* prints client's IP */

						clientInSystem[i].clientAddr = clientAddr;
						clientInSystem[i].socket = connSock;
						clientInSystem[i].isActive = true;
						clientInSystem[i].isLogin = false;
						clientInSystem[i].username = "";

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

			inet_ntop(AF_INET, &clientInSystem[index].clientAddr.sin_addr, clientIP, sizeof(clientIP));
			clientPort = ntohs(clientInSystem[index].clientAddr.sin_port);

			req = "";
			tstID = -1;
			ret = customRecv(clientInSystem[index].socket, tstID, req);

			if (ret <= 0) {
				//Release socket and event if an error occurs
				closeClientAndEvent(index, nEvents);
				nEvents--;
			}
			else {	
				cout << "Receive from client [" << clientIP << ":" << clientPort << "]: " << req << endl;

				int resp = (ret == CODE_PACKET_RECEIVE_ERROR 
					? CODE_ERROR_SYSTEM_DEFAULT : controllerServer(&clientInSystem[index], req));

				ret = customSend(clientInSystem[index].socket, tstID, to_string(resp));
				if (ret == SOCKET_ERROR)
					cout << "Error " << WSAGetLastError() 
						<< ": Cannot send data to client[" << clientIP << ":" << clientPort << "] !!!" << endl;

				logManager(&clientInSystem[index], req, resp);

				//reset event
				//WSAResetEvent(events[index]);
			}
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