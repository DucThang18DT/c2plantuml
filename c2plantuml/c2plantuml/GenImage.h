#pragma once
#include <iostream>
#include <vector>

class GenImage
{
public: 
	GenImage(std::string& outputDir);
	~GenImage();

	void GetListFile();

	void GenXmiFile(std::string& inUmlDir, std::string& inUmlFileName);
	void GenImageFromUml(std::string& inUmlDir, std::string& inUmlFileName);
private:
	std::vector<std::string> listFile;
	std::string umlDir;
	std::string imageDir;
	std::string outputDir;
};

