#include "stdafx.h"

// contruct client in the system 
typedef struct {
	bool isActive = false; // client is connect or not
	SOCKET socket = NULL; // socket of client
	sockaddr_in clientAddr; // address of client
	string username = ""; // username of account login on client
	bool isLogin = false; // client has account login or not
} Client;

CRITICAL_SECTION critical;
Client clientInSystem[MAX_CLIENT];

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

	int num_size = (int)log10(num) + 1;
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

	return rs;
}

/**
* @function customeRecv: receive a packet(transactionId - 10bytes, data size - 10bytes, data)
*						from client to get transactionID request and specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to receive data
* @param2 tstID_req: transaction ID from packet receive from client 
* @param3 data: the string receive data from client
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int customRecv(SOCKET &socket, int &tstID_req, string &data) {
	char buff[HEADER_SIZE + PADDING_BUFF];
	int rs = 0;
	data = "";

	rs = recv(socket, buff, HEADER_SIZE, MSG_WAITALL);

	if (rs > 0) {
		buff[rs] = 0;
		tstID_req = atoi(buff);

		memset(&buff, 0, sizeof(buff));
		rs = recv(socket, buff, HEADER_SIZE, MSG_WAITALL);

		if (rs > 0) {
			buff[rs] = 0;
			int data_size = atoi(buff);

			if (data_size > 0) {
				char *data_buff = new char[data_size + PADDING_BUFF];

				while (data_size > 0) {
					rs = recv(socket, data_buff, data_size, 0);
					if (rs == SOCKET_ERROR)
						break;
					if (rs > 0) {
						data_buff[rs] = 0;
						data.append(data_buff);
						data_size -= rs;
						memset(data_buff, 0, sizeof(data_buff));
					}
				}

				delete[] data_buff;

				if (rs == SOCKET_ERROR)
					return rs;

				int size_delim = DELIMITER.size();
				if (data.compare(data.size() - size_delim, size_delim, DELIMITER) != 0) 
					rs = CODE_PACKET_RECEIVE_ERROR;

				data = data.substr(0, data.size() - size_delim);
			}
		}
	}

	return rs;
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
	log	<< clientIP << ":" << clientPort << " " << getCurrentTimeInSystem() << " $ " << data << " $ " << rs;

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
* @function checkAccountOnlineOnAnotherDevice: check account is logged in the system or not
*
* @param username: username of account
*
* @return: account is logged in the system or not
**/
bool checkAccountOnlineOnAnotherDevice(string username) {
	for(int i = 0; i < MAX_CLIENT; i++){
		Client tmp = clientInSystem[i];
		if (tmp.isActive && tmp.isLogin)
			if (!tmp.username.compare(username))
				return true;
	}

	return false;
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

	bool checkAccOnAnotherDevice = checkAccountOnlineOnAnotherDevice(username);
	
	if (checkAccOnAnotherDevice)
		rs = CODE_LOGIN_FAIL_SIGNED_ANOTHER_DEVICE;
	else {
		int checkAcc = checkAccountInSystem(username);
		if (!checkAcc) {
			rs = CODE_LOGIN_SUCCESS;
			client->username = username;
			client->isLogin = true;
		}
		else {
			if(checkAcc != CODE_ERROR_SYSTEM_DEFAULT)
				rs = checkAcc > 0 ? CODE_LOGIN_FAIL_ACCOUNT_BLOCK : CODE_LOGIN_FAIL_NOT_EXIST_ACCOUNT;
		}
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
* @function echoThread: thread to receive the message from client and echo
*
* @param param: client receive from main thread
*
* @return: 0
**/
unsigned __stdcall echoThread(void *param) {
	string req;
	int tstID;
	int rs = -1;

	Client *client = (Client *)param;
	SOCKET connSock = client->socket;
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &client->clientAddr.sin_addr, clientIP, sizeof(clientIP));
	int clientPort = ntohs(client->clientAddr.sin_port);

	while (true) {
		rs = customRecv(connSock, tstID, req);
		if (rs == SOCKET_ERROR) {
			cout << "Error " << WSAGetLastError() << ": Cannot receive data from client[" << clientIP << ":" << clientPort << "] !!!" << endl;
			break;
		}
		else if (req.size() > 0) {
			cout << "Receive from client [" << clientIP << ":" << clientPort << "]: " << req << endl;

			EnterCriticalSection(&critical);
			int resp = (rs == CODE_PACKET_RECEIVE_ERROR ? CODE_ERROR_SYSTEM_DEFAULT : controllerServer(client, req));
			LeaveCriticalSection(&critical);

			rs = customSend(connSock, tstID, to_string(resp));
			if (rs == SOCKET_ERROR) {
				cout << "Error " << WSAGetLastError() << ": Cannot send data to client[" << clientIP << ":" << clientPort << "] !!!" << endl;
				break;
			}

			logManager(client, req, resp);
		}
		else {
			cout << "Client [" << clientIP << ":" << clientPort << "] disconnected." << endl << endl;
			break;
		}
	}

	client->isActive = false;
	client->socket = NULL;
	client->isLogin = false;
	client->username = "";
	memset(&client->clientAddr, 0, sizeof(client->clientAddr));

	// close connSock
	closesocket(connSock);
	return 0;
}


int main(int argc, char *argv[]) {
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
		cout << "Error " << WSAGetLastError() << ": Cannot create server socket!!!" << endl;
		return -1;
	}

	// Step 3: Bind address to socket
	const int SERVER_PORT = atoi(argv[1]);

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR.c_str(), &serverAddr.sin_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "Error " << WSAGetLastError() << ": Cannot associate a local address with server socket!!!" << endl;
		return -1;
	}

	// Step 4: Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		cout << "Error " << WSAGetLastError() << ": Cannot place server socket in state LISTEN!!!" << endl;
		return -1;
	}

	cout << "SERVER STARTED WITH SERVER_PORT: " << SERVER_PORT << " AND SERVER_ADDRESS: " << SERVER_ADDR << " !" << endl << endl;

	// Step 5: Communicate with client
	SOCKET connSock;
	sockaddr_in clientAddr;

	char clientIP[INET_ADDRSTRLEN];
	int clientAddrLen = sizeof(clientAddr), clientPort;

	// init queue client
	
	for (int i = 0; i < MAX_CLIENT; i++)
		memset(&clientInSystem[i].clientAddr, 0, sizeof(clientInSystem[i].clientAddr));

	HANDLE myhandle[MAX_CLIENT];

	while (true) {
		InitializeCriticalSection(&critical);
		for (int i = 0; i < MAX_CLIENT; i++) {
			if (!clientInSystem[i].isActive) {
				// Accept request from client
				connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
				if (connSock == SOCKET_ERROR)
					cout << "ERROR " << WSAGetLastError() << ": Cannot permit incoming connection!!!" << endl;
				else {
					inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
					clientPort = ntohs(clientAddr.sin_port);

					clientInSystem[i].clientAddr = clientAddr;
					clientInSystem[i].socket = connSock;
					clientInSystem[i].isActive = true;
					clientInSystem[i].isLogin = false;
					clientInSystem[i].username = "";

					cout << "Accept incoming connection from " << clientIP << ":" << clientPort << endl;

					myhandle[i] = (HANDLE)_beginthreadex(0, 0, echoThread, (void *)&clientInSystem[i], 0, 0); // start thread
				}

				break;
			}
		}

		WaitForMultipleObjects(MAX_CLIENT, myhandle, TRUE, INFINITE);
		DeleteCriticalSection(&critical);
	}

	// Step 6: Close socket
	closesocket(listenSock);

	// Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}