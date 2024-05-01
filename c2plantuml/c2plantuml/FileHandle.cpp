#include "FileHandle.h"
#include <string>
#include <filesystem>
FileHandle::FileHandle()
{
}

FileHandle::~FileHandle()
{
}

void FileHandle::Write(std::string& inFilePath, std::string& inContent)
{
	std::ofstream writeFile(inFilePath, std::ios::app | std::ios::in);
	writeFile << inContent << "\n";
	writeFile.close();
}

void FileHandle::Read(std::string& inFilePath, std::string& outContent)
{
	std::ifstream readFile(inFilePath, std::ios::out | std::ios::app);
	std::string lineContent;
	while (getline(readFile, lineContent))
	{
		outContent += lineContent + "\n";
	}
	readFile.close();
}

void FileHandle::CreateDir(std::string& inFilePath)
{
	if (!std::filesystem::exists(inFilePath))
	{
		std::filesystem::create_directory(inFilePath);
	}
}
