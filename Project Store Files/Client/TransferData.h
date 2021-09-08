// TransferData library for Client

#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <vector>
#include "utils.h"

#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define PADDING_BUFF		10
#define OPCODE_BLOCK_SIZE	1
#define OPCODE_MESSAGE_SIZE 4
#define LENGTH_BLOCK_SIZE	2
#define PAYLOAD_BLOCK_SIZE	16384
#define PACKET_SIZE			16400

#define FILE_NOT_FOUND		-404


struct Message {
	string opcode = "";
	int length = 0;
	string payload = "";

	// default constructor
	Message() {
		this->opcode = "";
		this->length = 0;
		this->payload = "";
	}

	// constructor
	Message(string _opcode, string _payload) {
		this->opcode = _opcode;
		this->length = _payload.size();
		this->payload = _payload;
	}

	string toPacket() {
		stringstream s;
		char length_enc[LENGTH_BLOCK_SIZE + 2];
		convertIntToChar(length, length_enc);
		s << opcode << length_enc << payload;

		return s.str();
	}
};


/**
* @function customeSend: send a packet(opcode - 4bytes, data size - 2bytes, data)
*						to client specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to send data
* @param2 msg: the message send to client
*
* @return: successul - number of bytes sent
*		   failed - SOCKET_ERROR
**/
int sendMess(SOCKET &socket, Message msg) {
	int rs = 0;
	string packet = msg.toPacket();

	// send packet(opcode , data_size, data) to client
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
* @function recvPartOfPacket: receive part of packet(opcode - 4bytes, data size - 2bytes, data)
*							from client
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
* @function customeRecv: receive a packet(opcode - 4bytes, data size - 2bytes, data)
*						from client to specify data size(create buff size) to receive data
*
* @param1 socket: socket connect with client to receive data
* @param2 msg: the message receive to client
*
* @return: successul - number of bytes received
*		   failed - SOCKET_ERROR
**/
int recvMess(SOCKET &socket, Message &msg) {
	int rs = 0;
	int bytes = 0;
	string opcode, length, payload;
	int data_size = 0;

	// receive opcode from packet
	rs = recvPartOfPacket(socket, opcode, OPCODE_MESSAGE_SIZE);

	if (rs > 0) {
		bytes += rs;

		// receive data size from packet
		rs = recvPartOfPacket(socket, length, LENGTH_BLOCK_SIZE);

		if (rs > 0) {
			bytes += rs;
			convertCharToInt(length.c_str(), data_size);

			if (data_size > 0) {
				rs = recvPartOfPacket(socket, payload, data_size);

				if (rs == SOCKET_ERROR)
					return rs;

				bytes += rs;
			}

		}
	}

	msg.opcode = opcode;
	msg.length = data_size;
	msg.payload = payload;

	return rs == SOCKET_ERROR ? rs : bytes;
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

	return (OPCODE_BLOCK_SIZE + LENGTH_BLOCK_SIZE + length);
}

/*
* Function		SendFile
* Description	send a file from client to server
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

	while ((bytes_read = fread(payload, sizeof(char), PAYLOAD_BLOCK_SIZE - 1, file)) > 0) {
		payload[bytes_read] = 0;
		ret = SendPacket(s, 2, bytes_read, payload);
		if (ret < 0) {
			fclose(file);
			return -1;
		}
		Sleep(20);
		ret = RecvPacket(s, opcode, length, payload, PAYLOAD_BLOCK_SIZE);
		if (ret < 0) {
			fclose(file);
			return -2;
		}
		if (!(opcode == 1 && length == 0)) {
			return -3;
		}
	}

	ret = SendPacket(s, 2, 0, "");
	if (ret < 0) {
		fclose(file);
		return -1;
	}

	ret = RecvPacket(s, opcode, length, payload, PAYLOAD_BLOCK_SIZE);
	if (ret < 0) {
		return -2;
	}
	if (!(opcode == 1 && length == 0)) {
		return -3;
	}

	fclose(file);
	return 0;
}

/*
* Function		RecvFile
* Description	receive a file which is sent from server
* Param s			A descriptor identifying a connected socket
* Param filePath	the directory where to save the file
* Return	error code
*/
int RecvFile(SOCKET s, string filePath) {
	int opcode = 2, length = 10;
	char payload[PAYLOAD_BLOCK_SIZE];
	while (opcode == 2 && length > 0) {
		int ret = RecvPacket(s, opcode, length, payload, PAYLOAD_BLOCK_SIZE);
		if (length > 0) {
			errno_t err;
			FILE *file;
			err = fopen_s(&file, filePath.c_str(), "ab");
			while (err != 0)
				err = fopen_s(&file, filePath.c_str(), "ab");
			fwrite(payload, sizeof(char), length, file);
			fclose(file);
			ret = SendPacket(s, 1, 0, "");
			if (ret < 0) {
				return -1;
			}
		}
	}
	return 0;
}

