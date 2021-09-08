#include "stdafx.h"

/**
* @function split: split a string into an array of data based on the delimiters
*
* @param data: the char delimeter used to split the string
*
* @return: vecto<string> after split
**/
vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

/**
* @function recvDataFromServer: print data receive from server to console
*
* @param data: the string receive from server
*
**/
void recvDataFromServer(string data) {
	if (data[0] == '-')
		cout << "Error!!! Your input is invalid (must only contains alphabet and number)" << endl;
	else if (data[0] == '+') {
		vector<string> rs = split(data, ' ');
		bool validData = false;
		int rs_size = rs.size();
		if (rs_size > 1) {
			for (int i = 1; i < rs_size; i++) {
				if (rs[i] != "") {
					if (!validData)
						cout << "Success!!! Respone from server: ";
					validData = true;
					cout << rs[i] << endl;
				}
			}

			if (!validData)
				cout << "Error!!! Your input is invalid (must only contains alphabet and number)" << endl;
		}
		else
			cout << "Error!!! Your input is invalid (must only contains alphabet and number)" << endl;
	}
}

/**
* @function customeSend: send a string(data size - 10bytes, data) to server specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with server to send data
* @param2 data: the string receive from input
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
	int d_size = data_size.size();
	int index = 0;

	// send string(data size, size) to client to create buff_size
	while (d_size > 0) { 
		rs = send(socket, data_size.c_str() + index, d_size, 0);
		d_size -= rs;
		index += rs;
	}

	return rs;
}

/**
* @function customeRecv: receive a string(data size - 10bytes, data) from server to specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with server to receive data
* @param2 data: the string receive data return from server
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
			while (buff_size > 0) {
				rs = recv(socket, tmp, buff_size, 0);
				if (rs > 0) {
					tmp[rs] = 0;
					data.append(tmp);
					buff_size -= rs;
					memset(tmp, 0, sizeof(tmp));
				}
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
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		cout << "Error " << WSAGetLastError() << ": Cannot creat server socket!!!" << endl;
		return -1;
	}

	cout << "Client started!" << endl;

	// Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	// step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		cout << "ERROR " << WSAGetLastError() << ": Cannot connect server!!!" << endl;
		return -1;
	}

	// Step 5: Communicate with server
	string data;
	int res, messLen;

	while (true) {
		// Send request to server
		cout << "Send to server: ";
		getline(cin, data);
		messLen = data.size();
		if (!messLen) // when input is '', exit program
			break;

		res = customeSend(client, data);
		if (res == SOCKET_ERROR)
			cout << "Error " << WSAGetLastError() << ": Cannot send data!!!" << endl;

		// Receive result from server
		res = customeRecv(client, data);
		if (res == SOCKET_ERROR) 
			cout << "Error " << WSAGetLastError() << ": Cannot receive data!!!" << endl;
		
		else if (res > 0) {
			// Processing result from server
			cout << "Receive from server: ";
			recvDataFromServer(data);
		}
	}

	// Step 5: Close socket
	closesocket(client);

	// Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}

