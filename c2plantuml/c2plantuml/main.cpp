#include "ParseSource.h"
#include "FileHandle.h"
#include <string>
#include "GenImage.h"
//#include<windows.h>  

int main(int arg, char* args[])
{
	std::string sourceDir = "E:\\VisualStudio\\C2Plantuml\\c2plantuml\\x64\\Debug\\src";
	if (args[1] != nullptr) {
		sourceDir = std::string(args[1]);
	}
	std::cout << "Source Dir: " << sourceDir << "\n";
	LogHandle* logFile = LogHandle::GetInstance(sourceDir);
	ParseSource parser(sourceDir);

	// check existing of source dir
	if (!parser.isExistDir(sourceDir))
	{
		std::cout << "Source directory does not exist (" << sourceDir <<")\n";
		system("pause");
		return 0;
	}
	WriteLogDebug("Dir exists");

	// Get list file in folder
	parser.GetListFile();
	int fileNum = parser.GetFileNum();
	WriteLogDebug("File num = " + std::to_string(fileNum));
	for (int index = 0; index < fileNum; index++)
	{
		std::string fileContent;
		parser.GetFileContent(index, fileContent);
		//WriteLogDebug(fileContent);
		parser.SeparateFile(fileContent);
	}
	bool retVal = true;
	FileHandle outFile;
	std::string outputDir = sourceDir + "\\output\\";
	outFile.CreateDir(outputDir);
	GenImage genImage(outputDir);
	while (retVal)
	{
		std::string umlSource;
		retVal = parser.GeneratePlantUmlSrc(umlSource);
		//WriteLogDebug("UML source: " + umlSource);
		if (retVal)
		{
			parser.OptimizeUmlSource(umlSource);
			logFile->WriteLogToFile("Uml source optimized: " + umlSource);
			std::string currentFuncName;
			parser.GetCurrentFuncName(currentFuncName);

			// TODO: write uml source to file
			std::string outpath = outputDir + currentFuncName + ".txt";
			outFile.Write(outpath, umlSource);

			//Sleep(1000);
			genImage.GenImageFromUml(outputDir, currentFuncName);
			genImage.GenXmiFile(outputDir, currentFuncName);
		}
	}
	system("pause");
	return 0;
}