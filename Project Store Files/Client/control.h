#pragma once
#include "TransferData.h"
using namespace std;

#define TRANSFER_ERROR -500


/*
* Function		SGINCmd
* Description	handle sign in request from user
* Param s			A descriptor identifying a connected socket
* Param username	username
* Param password	password
* Return	error code
*/
int SGINCmd(SOCKET s, string username, string password) {
	int ret;
	string data = username + " " + password;
	Message msg_send = Message("SGIN", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}


	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("SGIN") != 0) {
		return TRANSFER_ERROR;
	}

	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 4)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		SGUPCmd
* Description	handle sign up request from user
* Param s			A descriptor identifying a connected socket
* Param username	username
* Param password	password
* Return	error code
*/
int SGUPCmd(SOCKET s, string username, string password) {
	int ret;
	string data = username + " " + password;
	Message msg_send = Message("SGUP", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}


	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("SGUP") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		LISTCmd
* Description	handle show all files and folder in current folder request from user
* Param s			A descriptor identifying a connected socket
* Param folders		a string vector containing all subfolders' name
* Param files		a string vector containing all files' name
* Return	error code
*/
int LISTCmd(SOCKET s, vector<string>& folders, vector<string>& files) {
	int ret;
	folders.clear();
	files.clear();
	string data = "";
	Message msg_send = Message("LIST", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("LIST") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");
	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 1)
		return TRANSFER_ERROR;
	if (res == 0) {
		if (payload.size() > 1) {
			for (size_t i = 1; i < payload.size(); i++) {
				if (payload[i].size() < 4)
					continue;
				string prefix = payload[i].substr(0, 4);
				string name = payload[i].substr(4);
				if (prefix.compare("FLDR") == 0)
					folders.push_back(name);
				else if (prefix.compare("FILE") == 0)
					files.push_back(name);
			}
		}
	}

	return res;
}


/*
* Function		PWDRCmd
* Description	handle show full path of the current folder request from user
* Param s					A descriptor identifying a connected socket
* Param folderDirectory		returned current folder directory from server
* Return	error code
*/
int PWDRCmd(SOCKET s, string& folderDirectory) {
	int ret;
	string data = "";
	Message msg_send = Message("PWDR", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("PWDR") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 1)
		return TRANSFER_ERROR;
	if (res == 0) {
		folderDirectory = payload[1];
	}

	return res;
}


/*
* Function		MKDRCmd
* Description	handle create new subfolder request from user
* Param s				A descriptor identifying a connected socket
* Param folderName		new subfolder name
* Return	error code
*/
int MKDRCmd(SOCKET s, string folderName) {
	int ret;
	string data = folderName;
	Message msg_send = Message("MKDR", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("MKDR") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		CHDRCmd
* Description	handle open a folder request from user
* Param s				A descriptor identifying a connected socket
* Param folderName		folder need to access
* Return	error code
*/
int CHDRCmd(SOCKET s, string folderName) {
	int ret;
	string data = folderName;
	Message msg_send = Message("CHDR", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("CHDR") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		DNLDCmd
* Description	handle download file request from user
* Param s					A descriptor identifying a connected socket
* Param fileNameServer		file name which wanted to download from server
* Param filePath			the directory of downloading file in computer
* Return	error code
*/
int DNLDCmd(SOCKET s, string fileNameServer, string filePath) {
	int ret;
	string data = fileNameServer;
	Message msg_send = Message("DNLD", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("DNLD") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;
	else if (res > 0)
		return res;

	if (RecvFile(s, filePath) < 0)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		UPLDCmd
* Description	handle upload file request from user
* Param s					A descriptor identifying a connected socket
* Param filePath			the directory of uploading file in computer
* Param fileNameServer		name of file which will be saved in server
* Return	error code
*/
int UPLDCmd(SOCKET s, string filePath, string& fileNameServer) {
	int ret;

	string data = getFileFolderName(filePath);
	Message msg_send = Message("UPLD", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("UPLD") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 1)
		return TRANSFER_ERROR;
	else if (res > 0)
		return res;

	fileNameServer = payload[1];

	if (SendFile(s, filePath) < 0)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		REMVCmd
* Description	handle remove file or folder request from user
* Param s		A descriptor identifying a connected socket
* Param name	file or folder name in server which wanted to delete
* Return	error code
*/
int REMVCmd(SOCKET s, string name) {
	int ret;
	string data = name;
	Message msg_send = Message("REMV", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("REMV") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		GOBKCmd
* Description	handle go back to previous folder request from user
* Param s				A descriptor identifying a connected socket
* Param parentFolder	new current folder name
* Return	error code
*/
int GOBKCmd(SOCKET s, string& parentFolder) {
	int ret;
	string data = "";
	Message msg_send = Message("GOBK", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("GOBK") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 2)
		return TRANSFER_ERROR;

	if (res == 0)
		parentFolder = payload[1];

	return res;
}


/*
* Function		PASSCmd
* Description	handle change password request from user
* Param s				A descriptor identifying a connected socket
* Param currPassword	current password
* Param newPassword		new password
* Return	error code
*/
int PASSCmd(SOCKET s, string currPassword, string newPassword) {
	int ret;
	string data = currPassword + " " + newPassword;
	Message msg_send = Message("PASS", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("PASS") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 1)
		return TRANSFER_ERROR;

	return res;
}


/*
* Function		EXITCmd
* Description	handle log out request from user
* Param s	A descriptor identifying a connected socket
* Return	error code
*/
int EXITCmd(SOCKET s) {
	int ret;
	string data = "";
	Message msg_send = Message("EXIT", data);

	ret = sendMess(s, msg_send);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	Message msg_recv;
	ret = recvMess(s, msg_recv);
	if (ret < 0) {
		return TRANSFER_ERROR;
	}

	if (msg_recv.opcode.compare("EXIT") != 0) {
		return TRANSFER_ERROR;
	}
	vector<string> payload = split(msg_recv.payload, " ");

	int res = atoi(payload[0].c_str());
	if (res < 0 || res > 1)
		return TRANSFER_ERROR;

	return res;
}