#pragma once

#include <iostream>
#include <string.h>
#include <sstream>
#include <ctime>
#include <openssl/sha.h>
#include <iomanip>
#include <regex>

#include "constant.h"

#pragma warning(disable : 4996)

using namespace std;

/**
* Function:	convertIntToChar
* Description:	convert an int ( >=0 and <= 65535) to two char
* Param	n		the input number
* Param	encNum	the buffer containing result;
* Return		true if converting successfully
*/
bool convertIntToChar(int n, char *encNum) {
	if (n < 0 || n > 65535)
		return false;

	int firstByte = n % 256;
	int secondByte = (n - firstByte) / 256;
	encNum[0] = 48 + firstByte;
	encNum[1] = 48 + secondByte;
	encNum[2] = 0;
	return true;
}


/**
* Function:	convertCharToInt
* Description:	convert two first char of a char array to number
* Param	encNum	the input char aray;

* Return		true if converting successfully
*/
void convertCharToInt(const char* encNum, int& n) {

	int firstByte = encNum[0] - 48;
	int secondByte = encNum[1] - 48;

	if (firstByte < 0)
		firstByte = 256 + firstByte;
	if (secondByte < 0)
		secondByte = 256 + secondByte;
	n = secondByte * 256 + firstByte;
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
* @function sha256: cryptographic hash functions
*
* @param str: string need hash
*
* @return: string after hashing with sha256
**/
string sha256(const string str)
{
	unsigned char hash[SHA256_DIGEST_LENGTH];
	SHA256_CTX sha256;
	SHA256_Init(&sha256);
	SHA256_Update(&sha256, str.c_str(), str.size());
	SHA256_Final(hash, &sha256);
	stringstream ss;
	for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		ss << hex << setw(2) << setfill('0') << (int)hash[i];
	}
	return ss.str();
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
* @function checkPassword: check password is valid or not with condition is length mimimum(8)
*						must has one number, one alpha and one special character (not have whitespace)
*
* @param password: the string passord need to check
*
* @return: result after checking
**/
bool checkPassword(string password) {
	bool alpha_case = false;
	bool number_case = false;
	bool special_char = false;
	bool space_char = false;

	regex alpha_expression{ "[a-zA-Z]+" };
	regex number_expression{ "[\\d]+" };
	regex special_char_expression{ "[\\W]+" };
	regex space_char_expression{ "[\\s]+" };

	bool rs = false;

	if (password.length() >= LENGTH_MINIMUM_PASSWORD) {
		alpha_case = regex_search(password, alpha_expression);
		number_case = regex_search(password, number_expression);
		special_char = regex_search(password, special_char_expression);
		space_char = regex_search(password, space_char_expression);

		//like: sum_of_positive_results = 1 + 1 + 1 (true/false as an integer)
		int sum_of_positive_results = alpha_case + number_case + special_char;

		if (sum_of_positive_results == 3 && !space_char)
			rs = true;
	}

	return rs;
}


/**
* @function checkPasswordConfirm: check password and password confirm (user input) is equal or not
*
* @param1 password: the string passord
* @param1 password: the string password confirm
*
* @return: result after checking
**/
bool checkPasswordConfirm(string password, string password_confirm) {
	return !password.compare(password_confirm);
}


/**
* @function checkUsername: check username is valid or not with condition is length mimimum(6)
*						must only has a-z, A-z, 0-9 and '.' | '_'
*
* @param username: the string username need to check
*
* @return: result after checking
**/
bool checkUsername(string username) {
	regex username_expression{ "^[a-zA-Z0-9]+([._]?[a-zA-Z0-9]+)*$" };

	bool rs = false;

	if (username.length() >= LENGTH_MINIMUM_USERNAME)
		rs = regex_match(username, username_expression);

	return rs;
}


/**
* @function checkValidFolderName: check folder name is valid or not with condition is not have space
*
* @param name: the string folder/file name need to check
*
* @return: result after checking
**/
bool checkValidFileOrFolderName(string name) {
	bool space_char = false;

	regex space_char_expression{ "[\\s]+" };

	bool rs = false;

	if (name.length()) {
		space_char = regex_search(name, space_char_expression);

		if (!space_char)
			rs = true;
	}

	return rs;
}


/**
* @function join: convert list string to string with prefix and delim
*
* @param1 list: list string need to join
* @param1 delim: the character separate part of list
* @param1 prefix: prefix of list
*
* @return: string after converting
**/
string join(vector<string> list, string delim, string prefix) {
	string rs = "";

	for (auto it = list.begin(); it != list.end(); ++it)
		rs.append(prefix).append(*it).append(" ");

	rs = rs.substr(0, rs.size() - 1);

	return rs;
}