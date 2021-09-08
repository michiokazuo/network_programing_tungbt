#pragma once

#include "client.h"
#include "message.h"
#include "constant.h"
#include "database.h"
#include "TransferData.h"
#include "FileManagement.h"

/**
* @function checkAccountOnlineOnAnotherDevice: check account is logged in the system or not
*
* @param1 clientInSystem: array of client on system
* @param2 username: username of account
*
* @return: account is logged in the system or not
**/
bool checkAccountOnlineOnAnotherDevice(CLIENT clientInSystem[], string username) {
	for (int i = 1; i < WSA_MAXIMUM_WAIT_EVENTS; i++) {
		CLIENT tmp = clientInSystem[i];
		if (!tmp.free && tmp.isLogin)
			if (!tmp.username.compare(username))
				return true;
	}

	return false;
}


/**
* @function processReqSignUp: process login request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqSignUp(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (client->isLogin)
		rs = CODE_SIGN_UP_ALREADY_SIGNED;
	else {
		vector<string> req = split(data, " ");
		string username = req[0];
		string password = data.substr(username.size() + 1);
		ACCOUNT acc_signUp = ACCOUNT(username, password);

		rs = signUp(acc_signUp);
	}

	payload = to_string(rs);
	return rs;
}

/**
* @function processReqLogIn: process login request from client
*
* @param1 clientInSystem: array of client on system
* @param2 client: client send request to server
* @param3 data: the data of packet receive from client
* @param4 payload: data send to client
*
* @return: result code
**/
int processReqLogIn(CLIENT clientInSystem[], CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (client->isLogin)
		rs = CODE_SIGN_IN_ALREADY_SIGNED;
	else {
		vector<string> req = split(data, " ");
		string username = req[0];
		string password = data.substr(username.size() + 1);
		ACCOUNT acc_login = ACCOUNT(username, password);
		ACCOUNT user;

		bool checkAccOnAnotherDevice = checkAccountOnlineOnAnotherDevice(clientInSystem, username);

		if (checkAccOnAnotherDevice)
			rs = CODE_SIGN_IN_SIGNED_ANOTHER_DEVICE;
		else {
			rs = signIn(acc_login, user);
			if (rs == CODE_SIGN_IN_SUCCESS) {
				client->username = username;
				string folder;
				folder.append(ROOT_FOLDER).append("/" + user.rootFolder);
				client->rootPath = folder;
				client->currentPath = folder;
				client->isLogin = true;
			}
		}
	}

	payload = to_string(rs);
	return rs;
}


/**
* @function processReqLogOut: process log out request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqLogOut(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (client->isLogin) {
		*client = CLIENT(false, client->socket, client->clientPort, client->clientIP,
			false, "", "", "", NORMAL_TASK);
		rs = CODE_LOGOUT_SUCCESS;
	}
	else
		rs = CODE_LOGOUT_FAIL;

	payload = to_string(rs);
	return rs;
}


/**
* @function processReqChangePassword: process change password request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqChangePassword(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_CHANGE_PASSWORD_FAIL;
	else {
		vector<string> req = split(data, " ");
		string old_password = req[0];
		string new_password = data.substr(old_password.size() + 1);

		rs = changePasswd(ACCOUNT(client->username, old_password), new_password);
	}

	payload = to_string(rs);
	return rs;
}


/**
* @function processReqShowList: process show list files and folders request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqShowList(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_SHOW_LIST_FAIL;
	else {
		rs = CODE_SHOW_LIST_FAIL;
		vector<string> listFiles, listFolders;

		if (listFileFolder(client->currentPath, listFiles, listFolders)) {
			rs = CODE_SHOW_LIST_SUCCESS;

			string files = join(listFiles, " ", PREFIX_FILE);
			string folders = join(listFolders, " ", PREFIX_FOLDER);

			if (!files.empty())
				payload.append(files + " ");
			if (!folders.empty())
				payload.append(folders);
		}
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqShowRecentPath: process show recent path folder request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqShowRecentPath(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_SHOW_RECENT_PATH_FAIL;
	else {
		rs = CODE_SHOW_RECENT_PATH_FAIL;
		string clientPath;

		if (convertServerPathToClientPath(client->currentPath, client->rootPath, clientPath)) {
			rs = CODE_SHOW_RECENT_PATH_SUCCESS;
			payload = clientPath;
		}
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqCreateSubFolder: process create sub folder request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqCreateSubFolder(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_CREATE_SUB_FOLDER_FAIL;
	else {
		rs = CODE_CREATE_SUB_FOLDER_SUCCESS;
		payload = createFolder(client->currentPath, data, true);
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqAccessSubFolder: process access sub folder request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqAccessSubFolder(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_ACCESS_SUB_FOLDER_FAIL;
	else {
		rs = accessFileFolder(client->currentPath, data) ? CODE_ACCESS_SUB_FOLDER_SUCCESS : CODE_ACCESS_SUB_FOLDER_NOT_EXISTS;
	}

	payload = to_string(rs);
	return rs;
}


/**
* @function processReqGoBackPreviousFolder: process back the previous folder request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqGoBackPreviousFolder(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_BACK_PREVIOUS_FOLDER_FAIL;
	else {
		switch (goBack(client->currentPath, client->rootPath))
		{
		case 0:
			payload = !client->rootPath.compare(client->currentPath)
				? "MyFolder" : getFileFolderName(client->currentPath);
			rs = CODE_BACK_PREVIOUS_FOLDER_SUCCESS;
			break;
		case 1:
			rs = CODE_BACK_PREVIOUS_FOLDER_ROOT;
			break;
		default:
			rs = CODE_BACK_PREVIOUS_FOLDER_FAIL;
			break;
		}
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqRemoveFileOrfolder: process remove file or folder request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqRemoveFileOrfolder(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_REMOVE_FILE_FOLDER_FAIL;
	else {
		string pathF;
		pathF.append(client->currentPath).append("/" + data);
		rs = removeFileFolder(pathF) ? CODE_REMOVE_FILE_FOLDER_SUCCESS : CODE_REMOVE_FILE_FOLDER_NOT_EXISTS;
	}

	payload = to_string(rs);
	return rs;
}


/**
* @function processReqUploadFile: process upload file request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqUploadFile(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_UPLOAD_FILE_FAIL;
	else {
		rs = CODE_UPLOAD_FILE_SUCCESS;

		payload = createFileName(client->currentPath, data);
		client->currentPath.append("/" + payload);
		client->mode = RECEIVE_FILE;
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqUploadFile: process download file request from client
*
* @param1 client: client send request to server
* @param2 data: the data of packet receive from client
* @param3 payload: data send to client
*
* @return: result code
**/
int processReqDownloadFile(CLIENT *client, string data, string &payload) {
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!client->isLogin)
		rs = CODE_DOWNLOAD_FILE_FAIL;
	else {
		rs = CODE_DOWNLOAD_FILE_NOT_FOUND;

		string pathFile;
		pathFile.append(client->currentPath).append("/" + data);
		if (fs::exists(pathFile)) {
			rs = CODE_DOWNLOAD_FILE_SUCCESS;
			client->currentPath = pathFile;
		}
	}

	payload.insert(0, to_string(rs) + (payload.empty() ? "" : " "));
	return rs;
}


/**
* @function processReqUploadFile: process download file request from client
*
* @param1 clientInSystem: array of client on system
* @param2 client: client send request to server
* @param3 msg_recv: the message receive from client
* @param4 msg_send: the message send to client
* @param5 check_download: check if is function download file or not
*
* @return: result code
**/
int controllMessagse(CLIENT clientInSystem[], CLIENT *client, Message msg_recv, Message &msg_send, bool &check_download) {
	string opcode = msg_recv.opcode;
	string payload;
	int rs = CODE_ERROR_SYSTEM_DEFAULT;

	if (!opcode.compare(OPCODE_SIGN_UP)) {
		rs = processReqSignUp(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_SIGN_IN)) {
		rs = processReqLogIn(clientInSystem, client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_LOGOUT)) {
		rs = processReqLogOut(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_CHANGE_PASSWORD)) {
		rs = processReqChangePassword(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_SHOW_LIST)) {
		rs = processReqShowList(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_SHOW_RECENT_PATH)) {
		rs = processReqShowRecentPath(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_CREATE_SUB_FOLDER)) {
		rs = processReqCreateSubFolder(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_ACCESS_SUB_FOLDER)) {
		rs = processReqAccessSubFolder(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_BACK_PREVIOUS_FOLDER)) {
		rs = processReqGoBackPreviousFolder(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_REMOVE_FOLDER_FILE)) {
		rs = processReqRemoveFileOrfolder(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_UPLOAD_FILE)) {
		rs = processReqUploadFile(client, msg_recv.payload, payload);
	}
	else if (!opcode.compare(OPCODE_DOWNLOAD_FILE)) {
		rs = processReqDownloadFile(client, msg_recv.payload, payload);
		check_download = !rs;
	}
	else {
		opcode = OPCODE_ERROR;
		payload = opcode;
	}

	msg_send = Message(opcode, payload);
	return rs;
}