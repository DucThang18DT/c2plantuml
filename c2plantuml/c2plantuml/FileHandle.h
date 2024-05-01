#pragma once
#include <iostream>
#include <fstream>

class FileHandle
{
public:
	FileHandle();
	~FileHandle();

	void Write(std::string& inFilePath, std::string& inContent);
	void Read(std::string& inFilePath, std::string& outContent);
	void CreateDir(std::string& inFilePath);
private:
};

