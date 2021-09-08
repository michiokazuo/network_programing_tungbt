#include "stdafx.h"

int tstID_req = 0; // create transaction ID request for client

/**
* @function isNumber: check the input string is a numeric string or not  
*
* @param s: the input string
*
* @return: true or false
**/
bool isNumber(string s)
{
	int size = s.length();
	for (int i = 0; i < size; i++)
		if (!isdigit(s[i]))
			return false;

	return true;
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
* @function menuInSystem: show menu (includes functions provided by the system) for client 
*						and client wil choose one of these functions
*
* @return: the code of function user selected
**/
int menuInSystem() {
	string choose;
	cout << "********** MENU SYSTEM **********" << endl;
	cout << "1. Log In" << endl;
	cout << "2. Post Message" << endl;
	cout << "3. Log Out" << endl;
	cout << "4. Exit System" << endl << endl;
	cout << "Please select the function provied by the system." << endl;
	cout << "Your choice: ";

	getline(cin, choose);

	return isNumber(choose) ? atoi(choose.c_str()) : -1;
}

/**
* @function customeSend: send a packet(transactionId - 10bytes, data size - 10bytes, data) 
*						to server get transactionID to respone client and specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with server to send data
* @param2 data: the string receive from input
*
* @return: successul - number of bytes sent
*		   failed - SOCKET_ERROR
**/
int customSend(SOCKET &socket, string data) {
	int rs = 0;
	string packet;
	// data append the character end of packet
	data.append(DELIMITER);

	// append transaction ID - 10bytes
	tstID_req++;
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
*						from server to get transactionID respone and specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with server to receive data
* @param2 data: the string receive data return from server
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int customRecv(SOCKET &socket, string &data) {
	char buff[HEADER_SIZE + PADDING_BUFF];
	int tstID_resp = -1;
	int rs = 0;

	while (tstID_resp != tstID_req) { // check transactionId respone and request is equal or not to specify packet return
		data = "";
		rs = recv(socket, buff, HEADER_SIZE, MSG_WAITALL);

		if (rs > 0) {
			buff[rs] = 0;
			tstID_resp = atoi(buff);

			if (tstID_resp != tstID_req) // if transactionId respone and request is not equal, continue cathing another packet
				continue;

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
						break;

					int size_delim = DELIMITER.size();
					if (data.compare(data.size() - size_delim, size_delim, DELIMITER) != 0) {
						rs = 1;
						data = to_string(CODE_ERROR_SYSTEM_DEFAULT);
					}
					else
						data = data.substr(0, data.size() - size_delim);
				}
			}
		}

		if (rs == SOCKET_ERROR)
			break;
	}

	return rs;
}


/**
* @function customResult: show result from server after processing request from client 
*
**/
void customResult(int rs) {
	switch (rs) {
		case CODE_LOGIN_SUCCESS:
			cout << "Login Successful!!!" << endl;
			break;
		case CODE_LOGIN_FAIL_NOT_EXIST_ACCOUNT:
			cout << "Login Failed. Your account dosen't exist on system!!!" << endl;
			break;
		case CODE_LOGIN_FAIL_ACCOUNT_BLOCK:
			cout << "Login Failed. Your account has been locked!!!" << endl;
			cout << "Please use another account to continue or contact us for assistance!!!" << endl;
			break;
		case CODE_LOGIN_FAIL_ALREADY_SIGNED:
			cout << "Login Failed. This device already has an login account in system!!!" << endl;
			cout << "If you want to use another account, you must logout current account in system!!!" << endl;
			break;
		case CODE_LOGIN_FAIL_SIGNED_ANOTHER_DEVICE:
			cout << "Login Failed. Your account has been logged in another device!!!" << endl;
			cout << "If you want use this account in this device, you must logout in another device." << endl;
			break;
		case CODE_POST_SUCCESS:
			cout << "Post message Successful!!!" << endl;
			break;
		case CODE_POST_FAIL_NOT_SINGED:
			cout << "Post message Failed. You must login before posting message!!!" << endl;
			break;
		case CODE_LOGOUT_SUCCESS:
			cout << "Logout Successful!!!" << endl;
			break;
		case CODE_LOGOUT_FAIL_NOT_SINGED:
			cout << "Logout Failed. You haven't logged in yet!!!" << endl;
			break;
		default:
			cout << "Error! The system has some error. Please try again!!!" << endl;
			break;
	}
}

/**
* @function customProcessInFunction: process send packet to server and receive packet from server 
*
* @param1 socket: socket connect with server
* @param2 req: the string data send to server
*
**/
void customProcessInFunction(SOCKET &socket, string req) {
	string resp;
	int rs = customSend(socket, req);

	if (rs == SOCKET_ERROR)
		cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;

	rs = customRecv(socket, resp);
	if (rs == SOCKET_ERROR) {
		if (WSAGetLastError() == WSAETIMEDOUT)
			cout << "Time-out!!!" << endl;
		else
			cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
	}
	else if (rs > 0)
		customResult(atoi(resp.c_str()));
}

/**
* @function logIntoSystem: process function login from client
*
* @param socket: socket connect with server
*
**/
void logIntoSystem(SOCKET &socket) {
	string username;
	cout << "Username: ";
	getline(cin, username);

	string req = FORMAT_LOGIN;
	req.append(" ");
	req.append(username);

	customProcessInFunction(socket, req);
}

/**
* @function postMessToSystem: process function post message from client
*
* @param socket: socket connect with server
*
**/
void postMessToSystem(SOCKET &socket) {
	string mess;
	cout << "Message: ";
	getline(cin, mess);

	string req = FORMAT_POST_MESS;
	req.append(" ");
	req.append(mess);

	customProcessInFunction(socket, req);
}

/**
* @function logOutSystem: process function logout from client
*
* @param socket: socket connect with server
*
**/
void logOutSystem(SOCKET &socket) {
	string req = FORMAT_LOGOUT;

	customProcessInFunction(socket, req);
}

/**
* @function controllerClient: control system in client
*
* @param socket: socket connect with server
*
**/
void controllerClient(SOCKET &socket) {
	bool exit = false;
	int func = -1;

	cout << "Hello, Welcome to system!!!" << endl;
	while (!exit) {
		func = menuInSystem();

		switch (func) {
			case 1:
				logIntoSystem(socket);
				break;
			case 2:
				postMessToSystem(socket);
				break;
			case 3:
				logOutSystem(socket);
				break;
			case 4:
				exit = true;
				cout << "Thanks for using our system. See you soon!!!" << endl;
				break;
			default:
				cout << "Sorry, the function you want is currently not supported. Please select another function!!!" << endl;
				break;
		}

		cout << "================================" << endl;
	}
}

int main(int argc, char* argv[]) {
	// Step 1: Init Winsock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		cout << "Winsock 2.2 is not supported!!!" << endl;
		return -1;
	}

	// Step 2: Construct socket
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	cout << "Client started!" << endl;

	// (optional) Set time-out for receiving
	int tv = 10000; // Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *)(&tv), sizeof(int));

	// Step 3: Specify server address
	const string SERVER_ADDR = argv[1];
	const int SERVER_PORT = atoi(argv[2]);

	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR.c_str(), &serverAddr.sin_addr);

	// step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "ERROR " << WSAGetLastError() << ": Cannot connect server!!!" << endl;
		return -1;
	}

	cout << "Connected to Server[" << SERVER_ADDR << ":" << SERVER_PORT << "] !!!" << endl << endl;

	// Step 5: Communicate with server
	controllerClient(client);

	// Step 6: Close socket
	closesocket(client);

	// Step 7: Terminate Winsock
	WSACleanup();

	return 0;
}
