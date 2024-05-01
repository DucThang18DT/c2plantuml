#include "GenImage.h"
#include <filesystem>
GenImage::GenImage(std::string& outputDir)
{
	umlDir = outputDir /*+ "\\outputXmi"*/;
	imageDir = outputDir /*+ +"\\outputImage"*/;
	if (!std::filesystem::is_directory(umlDir))
	{
		std::filesystem::create_directory(umlDir);
	}
	if (!std::filesystem::is_directory(imageDir))
	{
		std::filesystem::create_directory(imageDir);
	}
}
GenImage::~GenImage()
{
}
void GenImage::GetListFile()
{
	struct stat sb;
	if (stat(umlDir.c_str(), &sb) == 0)
	{
		std::cout << "List files in the uml dir (" << umlDir << "): \n";
		for (const auto& index : std::filesystem::recursive_directory_iterator(umlDir))
		{
			if (!std::filesystem::is_directory(index.path()))
			{
				std::string filePath = index.path().string();
				if (filePath.find_first_of(".txt") != std::string::npos)
				{
					listFile.push_back(filePath);
					std::cout << filePath << "\n";
				}
			}
		}
	}
	else
	{
		std::cout << "Uml dir is not exist (" << umlDir << ")";
	}
}

void GenImage::GenXmiFile(std::string& inUmlDir, std::string& inUmlFileName)
{
	std::string umlFilePath = inUmlDir + inUmlFileName + ".txt";
	if (!std::filesystem::exists(umlFilePath))
	{
		std::cout << "uml file does not exist: " << umlFilePath << "\n";
		return;
	}
	std::string cmd = "java -jar \"C:\\Program Files\\plantuml\\plantuml.jar\" -xmi " + umlFilePath;
	system(cmd.c_str());
}

void GenImage::GenImageFromUml(std::string& inUmlDir, std::string& inUmlFileName)
{
	std::string umlFilePath = inUmlDir + inUmlFileName + ".txt";
	if (!std::filesystem::exists(umlFilePath))
	{
		std::cout << "uml file does not exist: " << umlFilePath << "\n";
		return;
	}
	std::string cmd = "java -jar \"C:\\Program Files\\plantuml\\plantuml.jar\" " + umlFilePath;
	system(cmd.c_str());
}
