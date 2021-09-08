#pragma once
#include "Control.h"
#include "TransferData.h"
#include <stdlib.h>

// define color code
#define BLACK			0x0
#define DARK_BLUE		0x1
#define DARK_GREEN		0x2
#define DARK_CYAN		0x3
#define DARK_RED		0x4
#define DARK_MAGENTA	0x5
#define DARK_YELLOW		0x6
#define LIGHT_GREY		0x7
#define GREY			0x8
#define BLUE			0x9
#define GREEN			0xA
#define CYAN			0xB
#define RED				0xC
#define MAGENTA			0xD
#define YELLOW			0xE
#define WHITE			0xF


void startService(SOCKET s, HANDLE hConsole);
void mainPageItf(SOCKET s, HANDLE hConsole, string username, string currentFolder); //DONE
void SGINItf(SOCKET s, HANDLE hConsole);
void SGUPItf(SOCKET s, HANDLE hConsole);
void LISTItf(SOCKET s, HANDLE hConsole);
void PWDRItf(SOCKET s, HANDLE hConsole);
void MKDRItf(SOCKET s, HANDLE hConsole);
void CHDRItf(SOCKET s, HANDLE hConsole, string username);
int DNLDITf_existFileSameName(HANDLE hConsole, string& fileNameClient, string folderPath, string& filePath);
void DNLDItf(SOCKET s, HANDLE hConsole, string username, string currentFolder);
void UPLDItf(SOCKET s, HANDLE hConsole, string username, string currentFolder);
void REMVItf(SOCKET s, HANDLE hConsole);
void GOBKItf(SOCKET s, HANDLE hConsole, string username);
void PASSItf(SOCKET s, HANDLE hConsole);
void EXITItf(SOCKET s, HANDLE hConsole);


/*
* Function		print
* Description	print string with specific text and background color
* Param hConsole		A handle to the console screen buffer
* Param text			print text
* Param textColor		text color
* Param textBackground	text background color
*/
void print(HANDLE hConsole, string text, int textColor, int textBackground) {
	int color = textBackground * 16 + textColor;
	SetConsoleTextAttribute(hConsole, color);
	cout << text;
	SetConsoleTextAttribute(hConsole, WHITE);
}


/*
* Function		startService
* Description	app startup interface
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void startService(SOCKET s, HANDLE hConsole) {
	system("CLS");
	cout << "         ";	print(hConsole, "******   FILE STORAGE SYSTEM   ******", RED, WHITE);	cout << "\n\n";
	print(hConsole, " 1. Sign in ", WHITE, DARK_CYAN);	cout << "\t\t";
	print(hConsole, " 2. Sign up ", WHITE, DARK_GREEN);	cout << "\t\t";
	print(hConsole, " 3. Exit ", WHITE, DARK_RED);		cout << "\n\n";
	print(hConsole, "Enter the number of mode you want to work with. \n", YELLOW, BLACK);
	print(hConsole, "For instance, enter '1' if you want to log in.\n\n", YELLOW, BLACK);

	bool modeSelected = false;
	while (!modeSelected) {
		print(hConsole, "Select Mode:", MAGENTA, GREY);	cout << " ";
		string modeStr;
		getline(cin, modeStr);
		if (!checkNumber(modeStr)) {
			print(hConsole, "Invalid mode!\n", RED, BLACK);
			continue;
		}

		int mode = atoi(modeStr.c_str());

		if (mode == 1) {
			modeSelected = true;
			SGINItf(s, hConsole);
		}
		else if (mode == 2) {
			modeSelected = true;
			SGUPItf(s, hConsole);
		}
		else if (mode == 3) {
			modeSelected = true;
			exit(0);
		}
		else {
			print(hConsole, "Invalid mode!\n", RED, BLACK);
		}
	}

}

/*
* Function		SGINItf
* Description	sign in interface
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void SGINItf(SOCKET s, HANDLE hConsole) {
	system("CLS");
	print(hConsole, "******   SIGN IN   ******", RED, WHITE);		cout << "\n\n";
	string username, password;

	while (true) {
		print(hConsole, "Username:", WHITE, DARK_CYAN);				cout << " ";
		getline(cin, username);
		if (!checkUsername(username)) {
			cout << "     ";
			print(hConsole, "Invalid Username", WHITE, RED);		cout << "\n";
			continue;
		}
		while (true) {
			print(hConsole, "Password:", MAGENTA, GREY);			cout << " ";
			getline(cin, password);
			if (!checkPassword(password)) {
				cout << "     ";
				print(hConsole, "Invalid Password", WHITE, RED);	cout << "\n";
				continue;
			}
			break;
		}
		break;
	}

	cout << "\n     ";

	int res = SGINCmd(s, username, password);

	if (res != 0) {
		if (res == 1) {
			print(hConsole, "Error. Another account is already signed in.", WHITE, RED);			cout << "\n";
		}
		else if (res == 2) {
			print(hConsole, "Error. Your username or password is incorrect.", WHITE, RED);			cout << "\n";
		}
		else if (res == 3) {
			print(hConsole, "Error. Your account has been locked.", WHITE, RED);					cout << "\n";
		}
		else if (res == 4) {
			print(hConsole, "Error. Your account has been logged in another device.", WHITE, RED);	cout << "\n";
		}
		else if (res == TRANSFER_ERROR) {
			print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);	cout << "\n";

		}

		Sleep(3000);
		startService(s, hConsole);
	}

	else {
		print(hConsole, "Login Successfully!", BLACK, CYAN); cout << "\n";
		Sleep(3000);
		mainPageItf(s, hConsole, username, "MyFolder");
	}
}


/*
* Function		SGUPItf
* Description	sign up interface
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void SGUPItf(SOCKET s, HANDLE hConsole) {
	system("CLS");
	print(hConsole, "******   SIGN UP   ******", RED, WHITE);	cout << "\n\n";
	print(hConsole, "     NOTICE     ", YELLOW, BLACK);			cout << "\n";
	print(hConsole, "- Username must contain only alphabets, digits and have at least 6 characters ", YELLOW, BLACK);	cout << "\n";
	print(hConsole, "- Password must contain at least a alphabet, a special character, not contain space and have at least 8 characters ", YELLOW, BLACK);	cout << "\n\n";


	string username, password1, password2;
	print(hConsole, "Your username:", WHITE, DARK_CYAN);		cout << " ";
	getline(cin, username);
	print(hConsole, "Your password:", MAGENTA, GREY);			cout << " ";
	getline(cin, password1);
	print(hConsole, "Re-type your password:", MAGENTA, GREY);	cout << " ";
	getline(cin, password2);
	cout << "\n     ";


	if (!checkUsername(username)) {
		print(hConsole, "Invalid Username!\n", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
		return;
	}

	if (!checkPassword(password1)) {
		print(hConsole, "Invalid Password!\n", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
		return;
	}

	if (!checkPasswordConfirm(password1, password2)) {
		print(hConsole, "Your Re-typed password is not the same as your password!\n", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
		return;
	}

	int res = SGUPCmd(s, username, password1);

	if (res == 0) {
		print(hConsole, "Sign up Successfully!", BLACK, CYAN);									cout << "\n";
	}
	else if (res == 1) {
		print(hConsole, "Error. Your username is already exist.", WHITE, RED);					cout << "\n";
	}
	else if (res == 2) {
		print(hConsole, "Error. Another account is already signed in.", WHITE, RED);			cout << "\n";
	}
	else if (res == TRANSFER_ERROR) {
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);	cout << "\n\n";
	}

	Sleep(3000);
	startService(s, hConsole);

}


/*
* Function		mainPageItf
* Description	main page interface after sign in successfully
* Param s				A descriptor identifying a connected socket
* Param hConsole		A handle to the console screen buffer
* Param username		username
* Param currentFolder	current folder name which user is in
*/
void mainPageItf(SOCKET s, HANDLE hConsole, string username, string currentFolder) {
	int cmdCount = 0; // count number of commands. Reset the screen after 5 commands
	bool firstTime = true;

	while (true) {
		if (cmdCount == 0) {
			if (!firstTime) {
				Sleep(3000);
			}
			firstTime = false;
			system("CLS");
			cout << "         "; print(hConsole, "******   FILE STORAGE SYSTEM   ******", RED, WHITE);	cout << "\n\n";
			print(hConsole, " User: ", MAGENTA, BLACK);
			print(hConsole, username, DARK_YELLOW, BLACK);												cout << "\t\t";
			print(hConsole, " Current Folder: ", MAGENTA, BLACK);
			print(hConsole, currentFolder, DARK_YELLOW, BLACK);											cout << "\n\n";
			print(hConsole, " 1. Show all files and folder in current folder ", WHITE, DARK_CYAN);		cout << "\n";
			print(hConsole, " 2. Show full path of the current folder ", WHITE, DARK_GREEN);			cout << "\n";
			print(hConsole, " 3. Create folder ", BLACK, YELLOW);										cout << "\t\t";
			print(hConsole, " 4. Open folder ", WHITE, DARK_YELLOW);									cout << "\n";
			print(hConsole, " 5. Download File ", WHITE, DARK_CYAN);									cout << "\t\t";
			print(hConsole, " 6. Upload File ", WHITE, DARK_GREEN);										cout << "\n";
			print(hConsole, " 7. Delete file/folder ", WHITE, RED);										cout << "\n";
			print(hConsole, " 8. Go back ", BLACK, CYAN);												cout << "\t\t";
			print(hConsole, " 9. Change password ", BLACK, CYAN);										cout << "\n";
			print(hConsole, " 10. Log out ", WHITE, DARK_MAGENTA);										cout << "\t\t";
			print(hConsole, " 11. Quit ", WHITE, DARK_RED);												cout << "\n\n";
			print(hConsole, "Enter the number of mode you want to work with. ", YELLOW, BLACK);			cout << "\n";
			print(hConsole, "For instance, enter '3' if you want to create a folder.", YELLOW, BLACK);	cout << "\n\n";
		}
		cmdCount = (cmdCount + 1) % 5;

		print(hConsole, "Select Mode:", MAGENTA, GREY);		cout << " ";
		string modeStr;
		getline(cin, modeStr);
		if (!checkNumber(modeStr)) {
			print(hConsole, "Invalid mode!", RED, BLACK);	cout << "\n";
			continue;
		}

		int mode = atoi(modeStr.c_str());

		switch (mode) {
			case 1:
			{
				LISTItf(s, hConsole);
				break;
			}
			case 2:
			{
				PWDRItf(s, hConsole);
				break;
			}
			case 3:
			{
				MKDRItf(s, hConsole);
				break;
			}
			case 4:
			{
				CHDRItf(s, hConsole, username);
				break;
			}
			case 5:
			{
				DNLDItf(s, hConsole, username, currentFolder);
				break;
			}
			case 6:
			{
				UPLDItf(s, hConsole, username, currentFolder);
				break;
			}
			case 7:
			{
				REMVItf(s, hConsole);
				break;
			}
			case 8:
			{
				GOBKItf(s, hConsole, username);
				break;
			}
			case 9:
			{
				PASSItf(s, hConsole);
				break;
			}
			case 10:
			{
				EXITItf(s, hConsole);
				break;
			}
			case 11: // turn off the client
			{
				exit(0);
				break;
			}
			default:
			{
				print(hConsole, "Invalid mode!", RED, BLACK);	cout << "\n";
				break;
			}
		}
	}
}


/*
* Function		LISTItf
* Description	interface for showing all files and folder in current folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void LISTItf(SOCKET s, HANDLE hConsole) {
	vector<string> folders;
	vector<string> files;
	int res = LISTCmd(s, folders, files);

	if (res == 0) {
		print(hConsole, "FOLDER: ", YELLOW, BLACK);		cout << "\t";
		for (size_t i = 0; i < folders.size(); i++) {
			print(hConsole, folders[i], WHITE, BLACK);	cout << "\t\t";
		}
		cout << "\n";

		print(hConsole, "FILE: ", BLUE, BLACK);
		for (size_t i = 0; i < files.size(); i++) {
			cout << "\t\t";
			print(hConsole, files[i], WHITE, BLACK);
		}
		cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);		cout << "\n\n";
	}
}


/*
* Function		PWDRItf
* Description	interface for showing full path of the current folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void PWDRItf(SOCKET s, HANDLE hConsole) {
	string folderDirectory;

	int res = PWDRCmd(s, folderDirectory);

	if (res == 0) {
		print(hConsole, "FOLDER PATH: ", DARK_YELLOW, BLACK);
		print(hConsole, folderDirectory, DARK_CYAN, BLACK);											cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);	cout << "\n\n";
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);		cout << "\n\n";
	}
}


/*
* Function		MKDRItf
* Description	interface for creating a folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void MKDRItf(SOCKET s, HANDLE hConsole) {
	string folderName;
	print(hConsole, "Notice, folder name contains only alphabets and digits", YELLOW, BLACK);	cout << "\n\n";
	print(hConsole, "Enter your new folder name:", WHITE, DARK_CYAN);							cout << " ";
	getline(cin, folderName);																	cout << "\n";

	if (!checkValidFileOrFolderName(folderName)) {
		cout << "     ";
		print(hConsole, "Error! Invalid folder name.", WHITE, RED);								cout << "\n\n";
		return;
	}

	int res = MKDRCmd(s, folderName);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Create Folder Successfully!", BLACK, CYAN);							cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == 2) {
		cout << "     ";
		print(hConsole, "Error. Your folder name is already exist.", WHITE, RED);				cout << "\n\n";
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);	cout << "\n\n";
	}
}


/*
* Function		CHDRItf
* Description	interface for opening a folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
* Param username	username
*/
void CHDRItf(SOCKET s, HANDLE hConsole, string username) {
	string folderName;
	print(hConsole, "Enter folder name:", WHITE, DARK_CYAN);									cout << " ";
	getline(cin, folderName);																	cout << "\n";

	if (!checkValidFileOrFolderName(folderName)) {
		cout << "     ";
		print(hConsole, "Error! Invalid folder name.", WHITE, RED);								cout << "\n\n";
		return;
	}

	int res = CHDRCmd(s, folderName);

	if (res == 0) {
		mainPageItf(s, hConsole, username, folderName);
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == 2) {
		cout << "     ";
		print(hConsole, "Error. Your folder name is not exist.", WHITE, RED);					cout << "\n\n";
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);	cout << "\n\n";
	}
}


/*
* Function		DNLDItf
* Description	interface for downloading a file
* Param s				A descriptor identifying a connected socket
* Param hConsole		A handle to the console screen buffer
* Param username		username
* Param currentFolder	current folder name which user is in
*/
void DNLDItf(SOCKET s, HANDLE hConsole, string username, string currentFolder) {
	system("CLS");
	cout << "         ";	print(hConsole, "******   FILE DOWNLOAD MANAGEMENT   ******", RED, WHITE);	cout << "\n\n";
	string fileNameServer, fileNameClient, folderPath, filePath;
	print(hConsole, "Download file:", WHITE, DARK_CYAN);								cout << " ";
	getline(cin, fileNameServer);
	if (fileNameServer.length() == 0)
		mainPageItf(s, hConsole, username, currentFolder);

	fileNameClient = fileNameServer;

	while (true) {
		print(hConsole, "Folder path to save file:", BLACK, YELLOW);					cout << " ";
		getline(cin, folderPath);
		if (!fs::exists(folderPath)) {
			cout << "     ";
			print(hConsole, "Error. Your folder path is not exist.", WHITE, RED);		cout << "\n\n";
			continue;
		}
		break;
	}

	if (folderPath[folderPath.size() - 1] != '/')
		folderPath = folderPath + "/";
	filePath = folderPath + fileNameClient;

	if (fs::exists(filePath)) {
		int mode = DNLDITf_existFileSameName(hConsole, fileNameClient, folderPath, filePath);
		if (mode == 3) {
			mainPageItf(s, hConsole, username, currentFolder);
		}
	}

	print(hConsole, "Downloading file ...", DARK_GREEN, BLACK);							cout << "\n\n";
	int res = DNLDCmd(s, fileNameServer, filePath);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Finish downloading file", WHITE, DARK_CYAN);					cout << "\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);		cout << "\n\n";

	}
	else if (res == 2) {
		cout << "     ";
		print(hConsole, "Error. Your file name is not exist in current folder in server.", WHITE, RED); cout << "\n\n";

	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);			cout << "\n\n";
	}

	Sleep(3000);
	mainPageItf(s, hConsole, username, currentFolder);
}


/*
* Function		DNLDITf_existFileSameName
* Description	interface for downloading file when there is already exist a file with
*				same name with file downloaded from server
* Param hConsole		A handle to the console screen buffer
* Param fileNameClient	the file name saved in client
* Param foderPath		the directory of folder in computer where you want to save your file
* Param filePath		the directory of file saved in computer
* Return	user choosen mode
*/
int DNLDITf_existFileSameName(HANDLE hConsole, string& fileNameClient, string folderPath, string& filePath) {
	print(hConsole, "There is already a file with the same name in this location.", RED, BLACK);		cout << "\n";
	cout << "\t\t"; print(hConsole, " 1. Change the name of downloading file ", WHITE, DARK_CYAN);		cout << "\n";
	cout << "\t\t"; print(hConsole, " 2. Replace the exist file with new file ", WHITE, DARK_GREEN);	cout << "\n";
	cout << "\t\t"; print(hConsole, " 3. Cancel downloading file", WHITE, DARK_RED);					cout << "\n\n";
	print(hConsole, "Enter the number of mode you want to work with.", YELLOW, BLACK);					cout << "\n";
	print(hConsole, "For instance, enter '3' if you want to cancel downloading file.", YELLOW, BLACK);	cout << "\n\n";

	int mode;
	while (true) {
		print(hConsole, "Select Mode:", MAGENTA, GREY);	cout << " ";
		string modeStr;
		getline(cin, modeStr);
		if (!checkNumber(modeStr)) {
			print(hConsole, "Invalid mode!\n", RED, BLACK);
			continue;
		}

		mode = atoi(modeStr.c_str());

		if (mode == 1) {
			while (true) {
				print(hConsole, "New filename: ", WHITE, DARK_CYAN);	cout << " ";
				getline(cin, fileNameClient);
				filePath = folderPath + fileNameClient;
				if (fs::exists(filePath)) {
					cout << "     ";  
					print(hConsole, "There is already a file with the same name in this location.", RED, BLACK);	cout << "\n";
					continue;
				}
				break;
			}
			return mode;
		}
		else if (mode == 2) {
			fs::remove_all(filePath);
			return mode;
		}
		else if (mode == 3) {
			return mode;
		}
		else {
			print(hConsole, "Invalid mode!", RED, BLACK);				cout << "\n";
		}
	}
}


/*
* Function		UPLDItf
* Description	interface for uploading a file
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
* Param username	username
* Param currentFolder	current folder name which user is in
*/
void UPLDItf(SOCKET s, HANDLE hConsole, string username, string currentFolder) {
	system("CLS");
	cout << "         ";	print(hConsole, "******   FILE UPLOAD MANAGEMENT   ******", RED, WHITE);	cout << "\n\n";
	string filePath;
	print(hConsole, "Directory of file you want to upload in your computer:", WHITE, DARK_CYAN);		cout << "\n->";
	getline(cin, filePath);
	if (filePath.length() == 0)
		mainPageItf(s, hConsole, username, currentFolder);

	while (!fs::exists(filePath)) {
		cout << "     ";
		print(hConsole, "Error. No such file or directory.", WHITE, RED);												cout << "\n\n";
		print(hConsole, "Please re-type the directory of file you want to upload in your computer:", WHITE, DARK_CYAN);	cout << "\n->";
		getline(cin, filePath);
		if (filePath.length() == 0)
			mainPageItf(s, hConsole, username, currentFolder);
	}

	print(hConsole, "Uploading file ...", DARK_GREEN, BLACK);			cout << "\n\n";

	string fileNameServer;
	int res = UPLDCmd(s, filePath, fileNameServer);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Finish uploading file. Your file saved in current folder is:", WHITE, DARK_CYAN);	cout << "  ";
		print(hConsole, fileNameServer, YELLOW, BLACK);														cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);			cout << "\n\n";

	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);				cout << "\n\n";
	}

	Sleep(3000);
	mainPageItf(s, hConsole, username, currentFolder);

}


/*
* Function		REMVItf
* Description	interface for removing a file or a folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void REMVItf(SOCKET s, HANDLE hConsole) {
	string name;
	print(hConsole, "Enter file/folder name:", WHITE, DARK_MAGENTA);		cout << " ";
	getline(cin, name);														cout << "\n";

	if (!checkValidFileOrFolderName(name)) {
		cout << "     ";
		print(hConsole, "Error! Invalid file/folder name.", WHITE, RED);	cout << "\n\n";
		return;
	}

	string validate;
	while (true) {
		print(hConsole, "Are you sure you want to delete file/folder ", YELLOW, BLACK);
		print(hConsole, name, CYAN, BLACK);
		print(hConsole, "? [Y/N]: ", YELLOW, BLACK);
		getline(cin, validate);
		if (validate.compare("N") == 0 || validate.compare("n") == 0) {
			cout << "\n";
			return;
		}
		else if (validate.compare("Y") == 0 || validate.compare("y") == 0)
			break;
	}

	int res = REMVCmd(s, name);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Delete File/Folder Successfully!", BLACK, CYAN);						cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == 2) {
		cout << "     ";
		print(hConsole, "Error. Your file/folder name is not exist.", WHITE, RED);				cout << "\n\n";
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);	cout << "\n\n";
	}
}


/*
* Function		GOBKItf
* Description	interface for going back to previous folder
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void GOBKItf(SOCKET s, HANDLE hConsole, string username) {
	string parentFolder;

	int res = GOBKCmd(s, parentFolder);

	if (res == 0) {
		mainPageItf(s, hConsole, username, parentFolder);
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == 2) {
		cout << "     ";
		print(hConsole, "YOU ARE IN ROOT FOLDER", BLACK, CYAN);										cout << "\n\n";
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);		cout << "\n\n";
	}
}


/*
* Function		PASSItf
* Description	interface for changing password
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void PASSItf(SOCKET s, HANDLE hConsole) {
	string curr_pass, password1, password2;

	print(hConsole, "Your current password:", DARK_YELLOW, BLACK);		cout << " ";
	getline(cin, curr_pass);
	if (!checkPassword(curr_pass)) {
		cout << "     ";
		print(hConsole, "Invalid Password!\n", WHITE, RED);				cout << "\n";
		return;
	}

	print(hConsole, "Your new password:", DARK_YELLOW, BLACK);			cout << " ";
	getline(cin, password1);
	if (!checkPassword(password1)) {
		cout << "     ";
		print(hConsole, "Invalid Password!\n", WHITE, RED);				cout << "\n";

		return;
	}

	print(hConsole, "Re-type your new password:", DARK_CYAN, BLACK);	cout << " ";
	getline(cin, password2);											cout << "\n";

	if (!checkPasswordConfirm(password1, password2)) {
		cout << "     ";
		print(hConsole, "Your Re-typed new password is not the same as your new password!", WHITE, RED);	cout << "\n\n";
		return;
	}

	int res = PASSCmd(s, curr_pass, password1);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Change Password Successfully!", BLACK, CYAN);								cout << "\n\n";
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);		cout << "\n\n";
	}

}


/*
* Function		EXITItf
* Description	interface for log out from current account
* Param s			A descriptor identifying a connected socket
* Param hConsole	A handle to the console screen buffer
*/
void EXITItf(SOCKET s, HANDLE hConsole) {
	int res = EXITCmd(s);

	if (res == 0) {
		cout << "     ";
		print(hConsole, "Logout Successfully!", BLACK, CYAN);										cout << "\n\n";
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == 1) {
		cout << "     ";
		print(hConsole, "Error. You are not logged in. Please log in and try again.", WHITE, RED);
		Sleep(3000);
		startService(s, hConsole);
	}
	else if (res == TRANSFER_ERROR) {
		cout << "     ";
		print(hConsole, "Error. An error occurred while processing your request.", WHITE, RED);		cout << "\n\n";
	}
}