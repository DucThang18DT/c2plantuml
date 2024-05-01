#pragma once
#include <iostream>
#include <vector>
#include <list>
#include "GenerateUML.h"
#include "LogHandle.h"

struct Depend
{
	std::string fileName; // source file name
	std::vector<std::string> funcsName; // func names in the file
};

struct FuncDefine
{
	std::string funcName; // name of function
	std::string funcDef; // definition of function (ex: abc; xyz();)
};

class ParseSource
{
public:
	ParseSource(std::string& inSourceDir);
	~ParseSource();

	// check if the directory is exist or not
	bool isExistDir(std::string& inDir);

	// Get list file in sourceDir
	void GetListFile();

	// get the number of files in source dir
	int GetFileNum();

	// Get all content of a file in list path
	void GetFileContent(int index, std::string& outContent);

	// separate file content and add depend list, func def list
	void SeparateFile(std::string& inFileContent);

	/* 
	* Generate Uml source for funcs in list func def.
	* Each time call this func, gen source for one of func in list def.
	* If return false, all of funcs in list are generated uml source
	*/
	bool GeneratePlantUmlSrc(std::string& outFuncUml);

	// Gen Uml source for a function
	void GeneratePlantUmlSrc(const std::string& inFuncDef, std::string& outFuncUml);

	// Get file name for current func that is generated uml source
	void GetCurrentFileName(std::string& outFileName);

	// Get current func name for uml source. Only call this func after GeneratePlantUmlSrc func
	void GetCurrentFuncName(std::string& outFuncName);

	// optimize the uml source
	void OptimizeUmlSource(std::string& inoutUmlSource);
	
private:

	// Get file name from path to file
	void GetFileName(const std::string& inFilePath, std::string& outFileName);

	// Get func definition (ex: func(){ abc; def;} => abc; def;)
	void GetFuncDef(const std::string& inFileName, const std::string& inFilecontent);

	// remove double space, tab, comment, preprocesor...
	void ContentFilter(std::string& inoutContent);

	// remmove double space, \n, spaces at begin and end of line
	void Filter(std::string& inoutContent);

	// Add a function name and the file name that contain the func to listDepend
	void AddDepend(const std::string inFileName,const std::string inFuncName);

	// Parse syntax of source and convert to Uml source
	void ParseSyntax(const std::string& inContent, bool inSwitchCase, std::string inSwitchVar, std::string& outUml);

	// parse syntax of command and function call and convert to Uml source
	void ParseCommandSyntax(const std::string& inCurrentClass, const std::string& inCommand, std::string& outResult);

	// check if a character is a special letter or not
	bool checkList(char inChar);

	// check if the func is exist on depend list or not
	bool isFuncNameExist(std::string& inFuncName);

	// Get file name that contain the function
	void GetDependency(std::string& inFuncName, std::string& outDepend);

	// check if func name is valid or not
	bool isFuncNameValid(const std::string& inFuncName);

	//  check if the content after the input con tent is in a string or not
	bool isInString(const std::string inContent);

	// Check if funcDef is empty or not
	bool checkValidFuncDef(std::string& inFuncDef);

	std::vector<Depend> listDepend; // list of files and funcs in file.
	std::list<FuncDefine> listFuncDef; // list of func definition
	std::vector<std::string> filePaths; // path to current file that is parsing
	std::string sourceDir; // path to source directory
	GenerateUML genUml; // gen Uml object
	std::string currentFile; // Name of current file that is parsing
	int funcIndex; // index of latest func that parsed.
	LogHandle* logFile;
};

