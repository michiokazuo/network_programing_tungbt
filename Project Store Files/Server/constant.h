#pragma once

// opcode of packet 
#define OPCODE_SIGN_UP "SGUP"
#define OPCODE_SIGN_IN "SGIN"
#define OPCODE_LOGOUT "EXIT"
#define OPCODE_CHANGE_PASSWORD "PASS"

#define OPCODE_SHOW_LIST "LIST"
#define OPCODE_SHOW_RECENT_PATH "PWDR"

#define OPCODE_CREATE_SUB_FOLDER "MKDR"
#define OPCODE_ACCESS_SUB_FOLDER "CHDR"
#define OPCODE_BACK_PREVIOUS_FOLDER "GOBK"
#define OPCODE_REMOVE_FOLDER_FILE "REMV"

#define OPCODE_UPLOAD_FILE "UPLD"
#define OPCODE_DOWNLOAD_FILE "DNLD"

#define OPCODE_ERROR "EROR"

// code respone from:
// function sign up
#define CODE_SIGN_UP_SUCCESS 0
#define CODE_SIGN_UP_ACCOUNT_ALREADY_EXISTS 1
#define CODE_SIGN_UP_ALREADY_SIGNED 2
#define CODE_SIGN_UP_FAIL 3

// function sign in
#define CODE_SIGN_IN_SUCCESS 0
#define CODE_SIGN_IN_ALREADY_SIGNED 1
#define CODE_SIGN_IN_NOT_EXIST_ACCOUNT 2
#define CODE_SIGN_IN_ACCOUNT_BLOCK 3
#define CODE_SIGN_IN_SIGNED_ANOTHER_DEVICE 4
#define CODE_SIGN_IN_FAIL 5

// function log out
#define CODE_LOGOUT_SUCCESS 0
#define CODE_LOGOUT_FAIL 1

// function change password
#define CODE_CHANGE_PASSWORD_SUCCESS 0
#define CODE_CHANGE_PASSWORD_FAIL 1

// function show list files and sub folder in recent folder 
#define CODE_SHOW_LIST_SUCCESS 0
#define CODE_SHOW_LIST_FAIL 1

// function create sub folder
#define CODE_CREATE_SUB_FOLDER_SUCCESS 0
#define CODE_CREATE_SUB_FOLDER_FAIL 1

// function access sub folder
#define CODE_ACCESS_SUB_FOLDER_SUCCESS 0
#define CODE_ACCESS_SUB_FOLDER_FAIL 1
#define CODE_ACCESS_SUB_FOLDER_NOT_EXISTS 2

// function go back previous folder
#define CODE_BACK_PREVIOUS_FOLDER_SUCCESS 0
#define CODE_BACK_PREVIOUS_FOLDER_FAIL 1
#define CODE_BACK_PREVIOUS_FOLDER_ROOT 2

// function show path of recent folder
#define CODE_SHOW_RECENT_PATH_SUCCESS 0
#define CODE_SHOW_RECENT_PATH_FAIL 1

// function upload file
#define CODE_UPLOAD_FILE_SUCCESS 0
#define CODE_UPLOAD_FILE_FAIL 1

// function download file
#define CODE_DOWNLOAD_FILE_SUCCESS 0
#define CODE_DOWNLOAD_FILE_NOT_FOUND 2
#define CODE_DOWNLOAD_FILE_FAIL 1

// function remove file or folder
#define CODE_REMOVE_FILE_FOLDER_SUCCESS 0
#define CODE_REMOVE_FILE_FOLDER_FAIL 1
#define CODE_REMOVE_FILE_FOLDER_NOT_EXISTS 2

// another code from server, default is system error
#define CODE_ERROR_SYSTEM_DEFAULT 500

// code from check account
#define CODE_ACCOUNT_INVALID -2
#define CODE_ACCOUNT_NOT_EXISTS -1
#define CODE_ACCOUNT_BLOCK 0
#define CODE_ACCOUNT_ACTIVE 1

#define DATABASE "account.xml"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 6000

#define LOG_SYSTEM "log.txt"

#define PADDING_BUFF 10

#define OPCODE_SIZE 4
#define LENGTH_SIZE 2

#define LENGTH_MINIMUM_PASSWORD 8
#define LENGTH_MINIMUM_USERNAME 6

#define ROOT_FOLDER "Clients Folders"

#define NORMAL_TASK -1
#define RECEIVE_FILE 0

#define PREFIX_FOLDER "FLDR"
#define PREFIX_FILE "FILE"

#define ROOT_ELEMENT_DB "Account"
#define ELEMENT_USER "User"
#define ELEMENT_USERNAME "Username"
#define ELEMENT_PASSWORD "Password"
#define ELEMENT_FOLDER "Folder"
#define ELEMENT_STATE "State"