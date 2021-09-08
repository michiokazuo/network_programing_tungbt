#pragma once

#include <iostream>
#include <stdlib.h> 
#include <string.h>
#include <sstream>

#include "tinyxml.h"

#include "utils.h"
#include "FileManagement.h"

using namespace std;

// create doc connect with database(file xml) for process
static TiXmlDocument doc(DATABASE);

// define struct for account of user when connect with server
struct ACCOUNT {
	string username;	// username of account
	string password;	// password of account	
	string rootFolder;	// root folder create for user

						// default constructor
	ACCOUNT() {
		this->password = "";
		this->username = "";
		this->rootFolder = "";
	}

	// constructor(username, password)
	ACCOUNT(string _username, string _password) {
		this->username = _username;
		this->password = _password;
	}

	// constructor all fields
	ACCOUNT(string _username, string _password, string _rootFolder) {
		this->username = _username;
		this->password = _password;
		this->rootFolder = _rootFolder;
	}
};


/**
* @function createNewDB: create new database(xml) when run server
*
* @return: result after creating
**/
bool createNewDB() {
	TiXmlDocument newDoc;

	TiXmlDeclaration *dec = new TiXmlDeclaration("1.0", "utf-8", "");
	newDoc.LinkEndChild(dec);

	TiXmlComment *cmt = new TiXmlComment("Database account for application store files");
	newDoc.LinkEndChild(cmt);

	TiXmlElement *root = new TiXmlElement("Account");
	newDoc.LinkEndChild(root);

	return newDoc.SaveFile(DATABASE);
}


/**
* @function createDB: create database(xml) when run server
*					if not exists, create new database
*					else if non create
*
* @return: result after creating
**/
bool createDB()
{
	bool rs = false;

	if (!doc.LoadFile())
	{
		cout << "Create new database" << endl;

		rs = createNewDB();
	}
	else
	{
		cout << "Database already exists!!!" << endl;
		rs = true;
	}

	return rs;
}


/**
* @function findUser: find user in database with username and password
*					if database has error format, create again
*
* @param1 root: element root of database (Account)
* @param2 acc: account receive from client
* @param3 user_find: user find from database
* @param4 signUp: true - if function call is sign up
*
* @return: state of account (invalid, not exists, block or active)
**/
int findUser(TiXmlElement *root, ACCOUNT acc, TiXmlElement **user_find, bool signUp)
{
	if (root == nullptr || (root != nullptr && strcmp(root->Value(), ROOT_ELEMENT_DB) != 0)) {
		cout << "Database has some error foramt!!! Create new DB!!!" << endl;
		createNewDB();

		doc.LoadFile();
		root = doc.RootElement();
	}

	int rs = CODE_ACCOUNT_INVALID; // account invalid
	string username = acc.username;
	string password = acc.password;
	if (!checkUsername(username) || !checkPassword(password))
		return rs;

	username = sha256(username);
	password = sha256(password);

	TiXmlElement *user = nullptr;
	rs = CODE_ACCOUNT_NOT_EXISTS; // not exists

	for (user = root->FirstChildElement(); user != NULL; user = user->NextSiblingElement())
	{
		TiXmlElement *usrname = user->FirstChildElement(ELEMENT_USERNAME);
		TiXmlElement *passwd = user->FirstChildElement(ELEMENT_PASSWORD);
		TiXmlElement *state = user->FirstChildElement(ELEMENT_STATE);

		if (!username.compare(usrname->GetText()) && signUp) {
			rs = atoi(state->GetText());
			break;
		}

		if (!username.compare(usrname->GetText()) && !password.compare(passwd->GetText()))
		{
			if (user_find != nullptr)
				*user_find = user;
			rs = atoi(state->GetText());
			break;
		}
	}

	return rs;
	// rs = 0 is block (CODE_ACCOUNT_BLOCK)
	// rs = 1 is active (CODE_ACCOUNT_ACTIVE)
}


/**
* @function signUp: sign up with account from client
*
* @param acc: account receive from client
*
* @return: status after processing
**/
int signUp(ACCOUNT acc)
{
	if (!doc.LoadFile())
	{
		cout << doc.ErrorDesc() << endl;
		createDB(); // create db again
	}

	string username = acc.username;
	string password = acc.password;
	string username_enc = sha256(username);
	string password_enc = sha256(password);

	TiXmlElement *root = doc.RootElement();
	int rs = findUser(root, acc, nullptr, true);

	if (rs != CODE_ACCOUNT_NOT_EXISTS)
	{
		return rs == CODE_ACCOUNT_INVALID ? CODE_SIGN_UP_FAIL : CODE_SIGN_UP_ACCOUNT_ALREADY_EXISTS;
		// account is block | already exists | invalid
	}

	// user
	TiXmlElement *user = new TiXmlElement(ELEMENT_USER);
	root->LinkEndChild(user);

	// username
	TiXmlElement *usrname = new TiXmlElement(ELEMENT_USERNAME);
	user->LinkEndChild(usrname);
	TiXmlText *val_usrname = new TiXmlText(username_enc.c_str());
	usrname->LinkEndChild(val_usrname);

	// password
	TiXmlElement *passwd = new TiXmlElement(ELEMENT_PASSWORD);
	user->LinkEndChild(passwd);
	TiXmlText *val_passwd = new TiXmlText(password_enc.c_str());
	passwd->LinkEndChild(val_passwd);

	// folder root
	// create folder
	string pathFolder = createFolder(ROOT_FOLDER, username_enc, false);

	TiXmlElement *folder = new TiXmlElement(ELEMENT_FOLDER);
	user->LinkEndChild(folder);
	TiXmlText *name_folder = new TiXmlText(username_enc.c_str());
	folder->LinkEndChild(name_folder);

	// state account - default signUp is 1, block is 0
	TiXmlElement *state = new TiXmlElement(ELEMENT_STATE);
	user->LinkEndChild(state);
	TiXmlText *val_state = new TiXmlText(to_string(CODE_ACCOUNT_ACTIVE).c_str());
	state->LinkEndChild(val_state);

	return doc.SaveFile() ? CODE_SIGN_UP_SUCCESS : CODE_SIGN_UP_FAIL;
}

/**
* @function signIn: sign in system with account from client
*
* @param1 acc: account receive from client
* @param2 acc: account from after login
*
* @return: status after processing
**/
int signIn(ACCOUNT acc, ACCOUNT &acc_login)
{
	if (!doc.LoadFile())
	{
		cout << doc.ErrorDesc() << endl;
		createDB(); // create db again
	}

	TiXmlElement *root = doc.RootElement();
	TiXmlElement *user = NULL;

	int rs = findUser(root, acc, &user, false);

	if (rs != CODE_ACCOUNT_ACTIVE)
	{
		switch (rs)
		{
		case CODE_ACCOUNT_BLOCK:
			rs = CODE_SIGN_IN_ACCOUNT_BLOCK;
			break;

		case CODE_ACCOUNT_NOT_EXISTS:
			rs = CODE_SIGN_IN_NOT_EXIST_ACCOUNT;
			break;

		default:
			rs = CODE_SIGN_IN_FAIL;
			break;
		}
		// account is block | not exists | invalid
	}
	else {
		rs = CODE_SIGN_IN_SUCCESS;
		TiXmlElement *usrname = user->FirstChildElement("Username");
		TiXmlElement *passwd = user->FirstChildElement("Password");
		TiXmlElement *folder = user->FirstChildElement("Folder");

		acc_login = ACCOUNT(usrname->GetText(), passwd->GetText(), folder->GetText());
	}

	return rs;
}


/**
* @function changePasswd: change password from request of user
*
* @param1 acc: account receive from client with old password
* @param2 new_password: new password need change
*
* @return: status after processing
**/
bool changePasswd(ACCOUNT acc, string new_password)
{
	if (!doc.LoadFile())
	{
		cout << doc.ErrorDesc() << endl;
		createDB(); // create db again
	}

	if (!checkPassword(new_password))
		return CODE_CHANGE_PASSWORD_FAIL; // new password invalid

	TiXmlElement *root = doc.RootElement();
	TiXmlElement *user = NULL;
	int rs = findUser(root, acc, &user, false);

	if (rs < 0)
	{
		return CODE_CHANGE_PASSWORD_FAIL;
		// account is not exists | invalid
	}

	// change password
	TiXmlElement *passwd = user->FirstChildElement("Password");
	passwd->Clear();
	TiXmlText *val_passwd = new TiXmlText(sha256(new_password).c_str());
	passwd->LinkEndChild(val_passwd);

	return doc.SaveFile() ? CODE_CHANGE_PASSWORD_SUCCESS : CODE_CHANGE_PASSWORD_FAIL;
}