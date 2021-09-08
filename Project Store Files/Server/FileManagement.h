#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

using namespace std;
namespace fs = std::experimental::filesystem;


/*
* Function		listFileFolder
* Description	get all files and subfolders name in a folder
* Param	folderPath		folder path
* Param	listFiles		a list containing all file names
* Param	listFolders		a list containing all folder names
* Return	true	- if getting successfully
*			false	- if the folderpath dont exist
*/
bool listFileFolder(const string folderPath, vector<string>& listFiles, vector<string>& listFolders) {
	if (!fs::exists(folderPath))
		return false;

	listFiles.clear();
	listFolders.clear();

	for (const auto & entry : fs::directory_iterator(folderPath)) {
		fs::path filePath(entry.path());
		string ext = filePath.extension().string();
		if (ext.compare("") == 0)
			listFolders.push_back(filePath.filename().string());
		else
			listFiles.push_back(filePath.filename().string());
	}

	return true;
}


/*
* Function		createFolder
* Description	create a subfolder in a folder
* Param	currentFolderPath		parent folder path
* Param	newFolderName			name of new folder
* Param preventDuplicate		true	- create new folder with new name if the parent folder
*										  already have a subfolder with same name.
*								false	- dont create new folder if the parent folder
*										  already have a subfolder with same name.
* Return	new subfolder name
*/
string createFolder(string currentFolderPath, string newFolderName, bool preventDuplicate) {
	if (currentFolderPath[currentFolderPath.size() - 1] != '/')
		currentFolderPath = currentFolderPath + "/";
	if (!fs::exists(currentFolderPath + newFolderName) || !preventDuplicate) {
		fs::create_directories(currentFolderPath + newFolderName);
		return newFolderName;
	}

	int index = 1;
	string newNameForFolder = newFolderName + "(" + to_string(index) + ")";
	while (fs::exists(currentFolderPath + newNameForFolder)) {
		index++;
		newNameForFolder = newFolderName + "(" + to_string(index) + ")";
	}
	fs::create_directories(currentFolderPath + newNameForFolder);
	return newNameForFolder;
}


/*
* Function		removeFileFolder
* Description	remove file or folder by its path
* Param	path			file/folder path
* Return	true	- if removing successfully
*			false	- if the path dont exist
*/
bool removeFileFolder(string path) {
	if (!fs::exists(path)) {
		return false;
	}

	fs::remove_all(path);
	return true;
}


/*
* Function		convertServerPathToClientPath
* Description	convert server path form to client path form
* Param	path			file/folder path form in server
* Param	rootPath		path of client's root folder in server
* Param clientPath		form of path in client
* Return	true	- if converting successfully
*			false	- if the path is not in rootpath
*/
bool convertServerPathToClientPath(string path, const string rootPath, string& clientPath) {
	if (path.size() < rootPath.size())
		return false;
	string root_path = path.substr(0, rootPath.size());
	if (root_path.compare(rootPath) != 0)
		return false;

	clientPath = path.substr(rootPath.size(), path.size() - rootPath.size());
	clientPath = "MyFolder" + clientPath;
	return true;
}


/*
* Function		goBack
* Description	change input path into parent path
* Param	path			input path
* Param	rootPath		path of root folder which will not be go back further than that
* Return	0	- if editing path successfully
*			1	- if input path is rootpath
*			-1	- if the path is not in rootpath
*/
int goBack(string& path, const string rootPath) {
	if (path.size() < rootPath.size())
		return -1;
	string root_path = path.substr(0, rootPath.size());
	if (root_path.compare(rootPath) != 0)
		return -1;

	fs::path currentFolderPath(path);
	fs::path rootFolderPath(rootPath);
	string currFolder = currentFolderPath.filename().string();
	string rootFolder = rootFolderPath.filename().string();
	if (currFolder.compare(rootFolder) == 0)
		return 1;

	path = currentFolderPath.parent_path().string();
	for (int i = 0; i < (int)path.size(); i++)
		if (path[i] == '\\')
			path[i] = '/';

	return 0;
}


/*
* Function		accessFileFolder
* Description	change input folder path into its file or subfolder path
* Param	path				input folder path
* Param	fileFolderName		file or subfolder name
* Return	true	- if editing path successfully
*			false	- if new directory dont exist
*/
bool accessFileFolder(string& path, string fileFolderName) {
	string newPath = path;
	if (newPath[newPath.size() - 1] != '/')
		newPath = newPath + "/";
	newPath = newPath + fileFolderName;

	if (!fs::exists(newPath))
		return false;
	else {
		path = newPath;
		return true;
	}
}

string createFileName(string currentFolderPath, string newFileName) {
	if (currentFolderPath[currentFolderPath.size() - 1] != '/')
		currentFolderPath = currentFolderPath + "/";
	if (!fs::exists(currentFolderPath + newFileName)) {
		return newFileName;
	}

	int index = 1;
	string newNameForFile = "(" + to_string(index) + ")" + newFileName;
	while (fs::exists(currentFolderPath + newNameForFile)) {
		index++;
		newNameForFile = "(" + to_string(index) + ")" + newFileName;
	}
	return newNameForFile;
}

/*
* Function		getFileFolderName
* Description	get file or folder name from its path
* Param	path	input file/folder path
* Return		file/folder name
*/
string getFileFolderName(string path) {
	fs::path fileFolderPath(path);
	return fileFolderPath.filename().string();
}