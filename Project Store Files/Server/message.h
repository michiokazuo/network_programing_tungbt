#pragma once

#include <iostream>
#include <string.h>
#include <winsock2.h>

#include "utils.h"
#include "constant.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

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

	// constructor(opcode, payload)
	Message(string _opcode, string _payload) {
		this->opcode = _opcode;
		this->length = _payload.size();
		this->payload = _payload;
	}

	/**
	* @function toPacket: convert from object Message to packet
	*
	* @return: packet after converting
	**/
	string toPacket() {
		stringstream s;
		char length_enc[LENGTH_SIZE + 2];
		convertIntToChar(length, length_enc);
		s << opcode << length_enc << payload;

		return s.str();
	}
};

/**
* @function sendMess: send a packet(opcode - 4bytes, data size - 2bytes, data)
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
* @function recvMess: receive a packet(opcode - 4bytes, data size - 2bytes, data)
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
	rs = recvPartOfPacket(socket, opcode, OPCODE_SIZE);

	if (rs == OPCODE_SIZE) {
		bytes += rs;

		// receive data size from packet
		rs = recvPartOfPacket(socket, length, LENGTH_SIZE);

		if (rs == LENGTH_SIZE) {
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