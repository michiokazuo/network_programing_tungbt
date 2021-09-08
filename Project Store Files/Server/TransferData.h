// TransferData library for Server

#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define OPCODE_BLOCK_SIZE 1
#define LENGTH_BLOCK_SIZE 2
#define PAYLOAD_BLOCK_SIZE 16384
#define PACKET_SIZE 16400
#define FILE_NOT_FOUND -404

/*
* @function copy: copy all characters from src to dest
* @param dest: array of chars destination
* @param posDest: position to start copy of array of chars destination
* @param src: array of chars need to copy
* @param lengthSrc: length of src
**/
void strcat_finetuned(char *strDestination, int EOSindex, char* strSource, int numberOfElements) {
	for (int i = EOSindex; i < EOSindex + numberOfElements; i++) {
		strDestination[i] = strSource[i - EOSindex];
	}
}


/*
* Function		SendPacket
* Description	send a packet [Opcode(1 byte) || Length(2 bytes) || Payload]
*				containing file data
* Param s		A descriptor identifying a connected socket
* Param opcode		opcode of the message
* Param length		size of payload in byte
* Param payload		the data to be transmitted
* Return	the data to be transmitted
*/
int SendPacket(SOCKET s, int opcode, int length, char* payload) {
	/**
	* The sent packet consist of 3 sections:
	* [message] = [Opcode(1 byte) || Length(2 bytes) || Payload]
	*/

	// Create opcode block
	string opcode_block;
	opcode_block = to_string(opcode);

	// Create length block
	string length_block;
	char length_enc[LENGTH_BLOCK_SIZE + 2];
	if (convertIntToChar(length, length_enc))
		length_block.append(length_enc);
	else
		return -2;

	// Create Packet
	char* packet = new char[PACKET_SIZE];
	memset(packet, 0, PACKET_SIZE);
	int ptr = 0;
	strcat_finetuned(packet, ptr, (char*)opcode_block.c_str(), OPCODE_BLOCK_SIZE);
	ptr += OPCODE_BLOCK_SIZE;
	strcat_finetuned(packet, ptr, length_enc, LENGTH_BLOCK_SIZE);
	ptr += LENGTH_BLOCK_SIZE;
	if (length > 0) {
		strcat_finetuned(packet, ptr, payload, length);
		ptr += length;
	}

	int size = ptr;


	// Send Packet
	int idx = 0;
	int nLeft = size;
	int ret = -1;
	while (nLeft > 0) {
		int ret = send(s, packet + idx, nLeft, 0);
		if (ret == SOCKET_ERROR)
			return ret;
		nLeft -= ret;
		idx += ret;
	}
	//cout << "send opcode: " << opcode << " length: " << length << endl;
	return size;
}


/*
* Function		RecvPacket
* Description	receive a packet [Opcode(1 byte) || Length(2 bytes) || Payload]
*				containing file data
* Param s				A descriptor identifying a connected socket
* Param opcode			opcode of the message
* Param length			length of received payload in byte
* Param payload			Location of the destination received payload buffer
* Param payLoadSize		Size of the destination received payload buffer in
*						char units for narrow and multi-byte functions
* Return	the data to be received
*/
int RecvPacket(SOCKET s, int& opcode, int& length, char* payload, int payLoadSize) {
	// get the opcode block
	int ret;
	char opcode_buff[OPCODE_BLOCK_SIZE + 1];
	int opcode_size = OPCODE_BLOCK_SIZE;
	while (opcode_size > 0) {
		ret = recv(s, opcode_buff, opcode_size, 0);
		if (ret < 0) return ret;
		if (ret > 0) {
			opcode_size -= ret;
			opcode_buff[ret] = 0;
			opcode = atoi(opcode_buff);
		}
	}

	// get data length block
	char length_buff[LENGTH_BLOCK_SIZE + 1];
	memset(length_buff, 0, LENGTH_BLOCK_SIZE + 1);
	char length_buff_part[LENGTH_BLOCK_SIZE + 1];
	int nLeft = LENGTH_BLOCK_SIZE;
	while (nLeft > 0) {
		ret = recv(s, length_buff_part, nLeft, 0);
		if (ret < 0) return ret;
		if (ret > 0) {
			nLeft -= ret;
			length_buff_part[ret] = 0;
			strcat_s(length_buff, LENGTH_BLOCK_SIZE + 1, length_buff_part);
		}
	}
	convertCharToInt(length_buff, length);

	// get payload block
	memset(payload, 0, payLoadSize);
	int ptr = 0;
	nLeft = length;
	if (nLeft > 0) {
		char *payLoad_buff_part = new char[PAYLOAD_BLOCK_SIZE];
		while (nLeft > 0) {
			ret = recv(s, payLoad_buff_part, nLeft, 0);
			if (ret < 0)
				return ret;
			if (ret > 0) {
				nLeft -= ret;
				payLoad_buff_part[ret] = 0;
				strcat_finetuned(payload, ptr, payLoad_buff_part, ret);
				ptr += ret;
			}
		}
		delete[] payLoad_buff_part;
	}

	//cout << "recv opcode: " << opcode << " length: " << length << endl;
	return (OPCODE_BLOCK_SIZE + LENGTH_BLOCK_SIZE + length);
}


/*
* Function		SendFile
* Description	send a file from server to client
* Param s			A descriptor identifying a connected socket
* Param filePath	the directory of file wanted to send
* Return	error code
*/
int SendFile(SOCKET s, string filePath) {
	int ret, bytes_read, opcode, length;
	char payload[PAYLOAD_BLOCK_SIZE];
	FILE *file;
	errno_t err = fopen_s(&file, filePath.c_str(), "rb");
	if (err != 0) {
		return FILE_NOT_FOUND;
	}

	cout << "Sending File To Server:";
	while ((bytes_read = fread(payload, sizeof(char), PAYLOAD_BLOCK_SIZE - 16, file)) > 0) {
		payload[bytes_read] = 0;
		ret = SendPacket(s, 2, bytes_read, payload);
		if (ret < 0) {
			cout << "Cannot send data to server\n";
			fclose(file);
			return -1;
		}
		Sleep(20);
		ret = RecvPacket(s, opcode, length, payload, PAYLOAD_BLOCK_SIZE);
		if (ret < 0) {
			cout << "Cannot receive ACK packet from server\n";
			fclose(file);
			return -2;
		}
		if (!(opcode == 1 && length == 0)) {
			cout << "An error has occor from server" << endl;
			return -3;
		}
	}

	ret = SendPacket(s, 2, 0, "");
	if (ret < 0) {
		cout << "Cannot send EOF packet to server " << ret << endl;
		fclose(file);
		return -1;
	}

	fclose(file);
	cout << "Finish sending file to client\n" << endl;
	return 0;
}


/*
* Function		RecvFile
* Description	receive a file which is sent from client
* Param s			A descriptor identifying a connected socket
* Param filePath	the directory where to save the file
* Param opcode			opcode of the message return by RecvPacket function
* Param length			length of received payload in byte return by RecvPacket function
* Param payload			Location of the destination received payload buffer return by RecvPacket function
* Param finish			return true if finish receiving file
* Return	error code
*/
int RecvFile(SOCKET s, string filePath, int opcode, int length, char* payload, bool& finish) {
	finish = false;
	if (length > 0) {
		errno_t err;
		FILE *file;
		err = fopen_s(&file, filePath.c_str(), "ab");
		while (err != 0)
			err = fopen_s(&file, filePath.c_str(), "ab");
		fwrite(payload, sizeof(char), length, file);
		fclose(file);
		int ret = SendPacket(s, 1, 0, "");
		if (ret < 0) {
			cout << "Cannot send data to client\n";
			return -1;
		}
	}
	else {
		int ret = SendPacket(s, 1, 0, "");
		if (ret < 0) {
			cout << "Cannot send EOF ACK packet to client\n";
			return -2;
		}
		cout << "Finish receive file from client\n" << endl;
		finish = true;
	}
	return 0;
}
