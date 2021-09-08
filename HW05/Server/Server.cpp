#include "stdafx.h"

#define WM_SOCKET (WM_USER + 1)

// contruct client in the system 
typedef struct {
	bool isActive = false; // client is connect or not
	SOCKET socket = NULL; // socket of client
	sockaddr_in clientAddr; // address of client
	string username = ""; // username of account login on client
	bool isLogin = false; // client has account login or not
} Client;

Client clientInSystem[MAX_CLIENT];
SOCKET listenSock;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
HWND				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	windowProc(HWND, UINT, WPARAM, LPARAM);
vector<string>		split(string, string);
const string		getCurrentTimeInSystem();
string				intToBytes(int, int);
int					customSend(SOCKET &, int, string);
int					recvPartOfPacket(SOCKET &, string &, int);
int					customRecv(SOCKET &, int &, string &);
void				logManager(Client *, string, int);
int					checkAccountInSystem(string);
int					processReqLogIn(Client *, string, string);
int					processReqPostMessage(Client *);
int					processReqLogOut(Client *);
int					controllerServer(Client *, string);

/*
*  FUNCTION: MyRegisterClass()
*
*  PURPOSE: Registers the window class.
*
*  COMMENTS:
*
*    This function and its usage are only necessary if you want this code
*    to be compatible with Win32 systems prior to the 'RegisterClassEx'
*    function that was added to Windows 95. It is important to call this function
*    so that the application will get 'well formed' small icons associated
*    with it.
*/
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SERVER));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

/*
*   FUNCTION: InitInstance(HINSTANCE, int)
*
*   PURPOSE: Saves instance handle and creates main window
*
*   COMMENTS:
*
*       In this function, we save the instance handle in a global variable and
*        create and display the main program window.
*/
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	int i;

	for (i = 0; i < MAX_CLIENT; i++) {
		clientInSystem[i].isActive = false;
		clientInSystem[i].socket = NULL;
		clientInSystem[i].isLogin = false;
		clientInSystem[i].username = "";
		memset(&clientInSystem[i].clientAddr, 0, sizeof(clientInSystem[i].clientAddr));
	}
		

	hWnd = CreateWindow(L"WindowClass", L"WSAAsyncSelect TCP Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return hWnd;
}

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
			rs = SOCKET_ERROR;
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

/*
*  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
*
*  PURPOSE:  Processes messages for the main window.
*
*  WM_SOCKET	- process the events on the sockets
*  WM_DESTROY	- post a quit message and return
*
*/
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int rs, clientAddrLen = sizeof(clientAddr), i;
	string req;
	int tstID;

	switch (message) {
		case WM_SOCKET:
			{
				if (WSAGETSELECTERROR(lParam)) {
					for (i = 0; i < MAX_CLIENT; i++)
						if (clientInSystem[i].isActive && clientInSystem[i].socket == (SOCKET)wParam) {
							closesocket(clientInSystem[i].socket);
							
							clientInSystem[i].isActive = false;
							clientInSystem[i].socket = NULL;
							clientInSystem[i].isLogin = false;
							clientInSystem[i].username = "";
							memset(&clientInSystem[i].clientAddr, 0, sizeof(clientInSystem[i].clientAddr));
							continue;
						}
				}

				switch (WSAGETSELECTEVENT(lParam)) {
					case FD_ACCEPT:
						{
							connSock = accept((SOCKET)wParam, (sockaddr *)&clientAddr, &clientAddrLen);

							if (connSock == INVALID_SOCKET) {
								break;
							}
							
							for (i = 0; i < MAX_CLIENT; i++)
								if (!clientInSystem[i].isActive) {
									clientInSystem[i].clientAddr = clientAddr;
									clientInSystem[i].socket = connSock;
									clientInSystem[i].isActive = true;
									clientInSystem[i].isLogin = false;
									clientInSystem[i].username = "";
									
									//requests Windows message-based notification of network events for listenSock
									WSAAsyncSelect(clientInSystem[i].socket, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);

									break;
								}

							if (i == MAX_CLIENT)
								MessageBox(hWnd, L"Too many clients!", L"Notice", MB_OK);
						}
						break;

					case FD_READ:
						{
							for (i = 0; i < MAX_CLIENT; i++)
								if (clientInSystem[i].isActive && clientInSystem[i].socket == (SOCKET)wParam)
									break;

							req = "";
							tstID = -1;

							connSock = clientInSystem[i].socket;

							rs = customRecv(connSock, tstID, req);

							if (rs > 0) {
								 //process request from client
								int resp = (rs == CODE_PACKET_RECEIVE_ERROR ? CODE_ERROR_SYSTEM_DEFAULT
									: controllerServer(&clientInSystem[i], req));

								rs = customSend(connSock, tstID, to_string(resp));

								logManager(&clientInSystem[i], req, resp);
							}

						}
						break;

					case FD_CLOSE:
						{
							for (i = 0; i < MAX_CLIENT; i++)
								if (clientInSystem[i].isActive && clientInSystem[i].socket == (SOCKET)wParam) {
									closesocket(clientInSystem[i].socket);

									clientInSystem[i].isActive = false;
									clientInSystem[i].socket = NULL;
									clientInSystem[i].isLogin = false;
									clientInSystem[i].username = "";
									memset(&clientInSystem[i].clientAddr, 0, sizeof(clientInSystem[i].clientAddr));
									break;
								}
						}
						break;
				}
			}
			break;

		case WM_DESTROY:
			{
				PostQuitMessage(0);
				shutdown(listenSock, SD_BOTH);
				closesocket(listenSock);
				WSACleanup();
				return 0;
			}
			break;

		case WM_CLOSE:
			{
				DestroyWindow(hWnd);
				shutdown(listenSock, SD_BOTH);
				closesocket(listenSock);
				WSACleanup();
				return 0;
			}
			break;
		}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND serverWindow;

	//Registering the Window Class
	MyRegisterClass(hInstance);

	//Create the window
	if ((serverWindow = InitInstance(hInstance, nCmdShow)) == NULL)
		return FALSE;

	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		MessageBox(serverWindow, L"Winsock 2.2 is not supported.", L"Error!", MB_OK);
		return 0;
	}

	//Construct socket	
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR.c_str(), &serverAddr.sin_addr);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot associate a local address with server socket.", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot place server socket in state LISTEN.", L"Error!", MB_OK);
		return 0;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}