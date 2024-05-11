#include "ParseSource.h"
#include <filesystem>
#include <fstream>
#include <string>
#include <sstream>
#include <sys/stat.h>
using namespace std;

#define DEF_STR	"#define"
#define UNDEF_STR "#undef"
#define INC_STR	"#include"
#define	NAMESPACE_STR	"namespace"
#define INLINE_STR	"inline"
#define LINE_CMT_STR	"//"
#define BEGIN_BLK_CMT_STR	"/*"
#define END_BLK_CMT_STR	"*/"
#define ERR_FUNCNAME	"FUNCNAME_ERR"
#define PLANT_UML_HEADER	"\n@startuml\n"
#define PLANT_UML_FOOTER	"\n@enduml\n"
#define FILE_EXTENSION		".c"

struct FuncProto
{
	std::string funcName;
	std::string params;
	int startPos;
	int endPos;
};

ParseSource::ParseSource(std::string& inSourceDir)
{
	WriteLogDebug("init Parser\n");
	sourceDir = inSourceDir;
	funcIndex = 0;
	logFile = LogHandle::GetInstance(sourceDir);
}

ParseSource::~ParseSource()
{
	logFile = nullptr;
}

bool ParseSource::isExistDir(std::string& inDir)
{
	struct stat sb;
	if (stat(sourceDir.c_str(), &sb) == 0) return true;
	return false;
}

void ParseSource::GetFileContent(int index, std::string& outContent)
{
	if (index >= filePaths.size()) return;
	outContent.clear();

	std::ifstream sourceFile(filePaths.at(index));
	string content;
	while (getline(sourceFile, content))
	{
		outContent += (content + "\n");
	}
	sourceFile.close();
	GetFileName(filePaths.at(index), currentFile);
}

void ParseSource::SeparateFile(std::string& inFileContent)
{
	ContentFilter(inFileContent);
	logFile->WriteLogToFile("content of file after filter: \n" + inFileContent);
	GetFuncDef(currentFile, inFileContent);
}

bool ParseSource::GeneratePlantUmlSrc(std::string& outFuncUml)
{
	//WriteLogDebug("Gen uml source");
	if (funcIndex >= listFuncDef.size()) return false;
	list<FuncDefine>::iterator iter = listFuncDef.begin();
	advance(iter, funcIndex);

	GetDependency(iter->funcName, currentFile);
	//WriteLogDebug("Gen uml source - File name: " + currentFile + "func: " + iter->funcName);
	
	GeneratePlantUmlSrc(iter->funcDef, outFuncUml);
	
	++funcIndex;
	return true;
}

void ParseSource::GeneratePlantUmlSrc(const std::string& inFuncDef, std::string& outFuncUml)
{
	string plantUMLSrc = PLANT_UML_HEADER;
	plantUMLSrc += ("->" + currentFile + "++:\n");
	outFuncUml += plantUMLSrc;
	ParseSyntax(inFuncDef, false, "", outFuncUml);
	plantUMLSrc = ("\n<--" + currentFile + "--:");
	plantUMLSrc += PLANT_UML_FOOTER;
	outFuncUml += plantUMLSrc;
}

void ParseSource::GetCurrentFileName(std::string& outFileName)
{
	outFileName = currentFile;
}

void ParseSource::GetCurrentFuncName(std::string& outFuncName)
{
	list<FuncDefine>::iterator iter = listFuncDef.begin();
	advance(iter, funcIndex -1);
	if (iter != listFuncDef.end())
	{
		outFuncName = iter->funcName;
	}
}

void ParseSource::OptimizeUmlSource(std::string& inoutUmlSource)
{
	genUml.OptimizeUmlSource(inoutUmlSource);
}

void ParseSource::ParseSyntax(const std::string& inContent, bool inSwitchCase, std::string inSwitchVar, std::string& outUml)
{
	unsigned long long index = 0;
	unsigned long long start = 0;

	bool isFirstCase = true;
	while (index < inContent.length())
	{
		if (inSwitchCase) // Process for parsing switch-case
		{
			int caseIdx= inContent.find("case ", index);
			if (caseIdx == string::npos)
			{
				caseIdx = inContent.find("defaul", index);

				if (caseIdx == string::npos)
				{
					cout << "parse switch-case failed - Cannot detect cases\n";
				}
				else
				{
					// Check if the "default" is in a string or not
					if (isInString(inContent.substr(0, caseIdx))) continue;

					index = caseIdx +1;
					start = caseIdx;
					int bracketCnt = -1;
					while (caseIdx < inContent.length())
					{
						++caseIdx;
						if ((inContent[caseIdx] == '{') && !isInString(inContent.substr(0, caseIdx))) --bracketCnt;
						if ((inContent[caseIdx] == '}') && !isInString(inContent.substr(0, caseIdx))) ++bracketCnt;
					}
					if (!bracketCnt)
					{
						while (index < inContent.length())
						{
							if (inContent[index] == ':')
							{
								if ((inContent[index - 1] != ':') && (inContent[index + 1] != ':') && !isInString(inContent.substr(0, index)))
								{
									string statement = inContent.substr(start, index - start);
									string statementUML;
									bool isSwitchCase;
									genUml.Statement(statement, isFirstCase, statementUML, isSwitchCase, inSwitchVar);
									outUml += statementUML + "\n";
									start = index + 1;

									string scopeSrc = inContent.substr(start);
									// Continue parse scopeSrc
									ParseSyntax(scopeSrc, false, "", outUml);
									if ((!isSwitchCase) && statementUML.length()) 
									{
										outUml += "END\n";
										isFirstCase = false;
									}
								}
							}
							++index;
						}
					}
				}
			}
			else
			{
				// Check if the "case " is in a string or not
				if (isInString(inContent.substr(0, caseIdx))) continue;

				index = caseIdx;
				start = caseIdx;
				while (index < inContent.length())
				{	
					++index;
					if (inContent[index] == ':')
					{
						if ((inContent[index - 1] != ':') && (inContent[index + 1] != ':') && (!isInString(inContent.substr(0, index))))
						{
							string statement = inContent.substr(start, index - start);
							string statementUML;
							bool isSwitchCase;
							genUml.Statement(statement,isFirstCase, statementUML, isSwitchCase, inSwitchVar);
							outUml += statementUML + "\n";
							start = index + 1;
							caseIdx = start-1;

							while (caseIdx < inContent.length())
							{
								// find the next case/ default keyword
								caseIdx = inContent.find("case ", caseIdx+1);

								if (caseIdx != string::npos)
								{
									if (isInString(inContent.substr(0, caseIdx))) continue;

									int bracketCnt = 0;
									for (int idx = start; idx <= caseIdx; idx++)
									{
										if ((inContent[idx] == '{') && !isInString(inContent.substr(0, idx))) ++bracketCnt;
										if ((inContent[idx] == '}') && !isInString(inContent.substr(0, idx))) --bracketCnt;
									}
									if (!bracketCnt) 
									{
										index = caseIdx;
										break;
									}
								}
								else
								{
									caseIdx = start - 1;
									int isOK = false;
									while (caseIdx < inContent.length())
									{
										caseIdx = inContent.find("default", caseIdx +1);

										if (caseIdx != string::npos)
										{
											if (isInString(inContent.substr(0, caseIdx))) continue;

											int bracketCnt = 0;
											for (int idx = start; idx <= caseIdx; idx++)
											{
												if ((inContent[idx] == '{') && (!isInString(inContent.substr(0, idx)))) ++bracketCnt;
												if ((inContent[idx] == '}') && (!isInString(inContent.substr(0, idx)))) --bracketCnt;
											}
											if (!bracketCnt)
											{
												index = caseIdx;
												isOK = true;
												break;
											}
										}
										else
										{
											index = inContent.length();
											isOK = true;
											break;
										}
									}
									if (isOK) break;
								}
							}
							string scopeSrc = inContent.substr(start, index - start);
							start = index;
							++index;
							// Continue parse scopeSrc
							ParseSyntax(scopeSrc, false, "", outUml);
							if ((!isSwitchCase) && statementUML.length()) { 
								outUml += "END\n"; 
								isFirstCase = false; 
							}
						}
					}
				}
			}
		}
		else // process for normal
		{
			//WriteLogDebug("parse syntax - index: " + to_string(index));
			if ((inContent[index] == '{') && !isInString(inContent.substr(0, index)))
			{
				string statement = inContent.substr(start, index - start);
				string statementUML;
				bool isSwitchCase;
				genUml.Statement(statement, false, statementUML, isSwitchCase, inSwitchVar);
				outUml += statementUML + "\n";
				// get code in scope
				++index;
				start = index;
				int curlyBracketCnt = 1;
				while ((curlyBracketCnt != 0) && (index < inContent.length()))
				{
					if ((inContent[index] == '{') && !isInString(inContent.substr(0, index))) ++curlyBracketCnt;
					if ((inContent[index] == '}') && !isInString(inContent.substr(0, index))) --curlyBracketCnt;
					++index;
				}
				if (curlyBracketCnt != 0)
				{
					cout << "Parse func error: no find the end of scope " << statement << endl;
				}
				else {
					string scopeSrc = inContent.substr(start, index - start - 1);
					start = index;
					// Continue parse scopeSrc
					//WriteLogDebug("Parse sub content - " + scopeSrc);
					ParseSyntax(scopeSrc, isSwitchCase, inSwitchVar, outUml);
					//WriteLogDebug("End parse sub content ");
				}
				if ((!isSwitchCase) && statementUML.length()) outUml += "END\n";
			}
			else if ((inContent[index] == ';') && !isInString(inContent.substr(0, index)))
			{
				// Process for command
				string command = inContent.substr(start, index - start + 1);
				string commandUml;
				ParseCommandSyntax(currentFile, command, commandUml);
				outUml += commandUml + "\n";
				start = index + 1;
				string remainCommand = inContent.substr(start);
				//ParseSyntax(remainCommand, outUml);
			}
			/*else if ((inContent[index] == '#') && !isInString(inContent.substr(0, index)))
			{
				unsigned long long _start = index;
				unsigned long long _end = index+1;
				int _preCnt = 0;
				while (index < inContent.length())
				{
					if ((inContent[index] == ' ') || (inContent[index] == '\n'))
					{
						break;
					}
					++index;
				}
				string preStr = inContent.substr(_start, index - _start);
				if ((preStr == "#if") || (preStr == "#ifdef") || (preStr == "#ifndef"))
				{
					_preCnt = 1;
				}
				while ((_preCnt != 0) && (_end < inContent.length()))
				{
					
					if ((inContent[index] == '{') && !isInString(inContent.substr(0, index))) ++curlyBracketCnt;
					if ((inContent[index] == '}') && !isInString(inContent.substr(0, index))) --curlyBracketCnt;
					++index;
				}
			}*/
			++index;
		}
	}
	outUml += "\n \n";
}

void ParseSource::ParseCommandSyntax(const std::string& inCurrentClass, const std::string& inCommand, std::string& outResult)
{	
	int start = 0;
	int index = 0;
	vector<FuncProto> listFunc;
	while (index < inCommand.length())
	{
		if ((inCommand[index] == '(') && !isInString(inCommand.substr(0, index)))
		{
			int end = index - 1;
			while (end >= start)
			{
				if (inCommand[end] == ' ')
				{
					--end;
				}
				else
				{
					break;
				}
			}
			int begin = end;
			while (begin >= start)
			{
				if (!checkList(inCommand[begin])) 
				{
					--begin;
				}
				else
				{
					break;
				}
			}
			std::string funcName = inCommand.substr(begin + 1, end - begin);
			
			if (isFuncNameExist(funcName))
			{
				FuncProto funcProt;
				funcProt.funcName = funcName;
				funcProt.startPos = begin;

				int bracketCnt = 1;
				while ((index < inCommand.length()) && bracketCnt)
				{
					++index;
					if ((inCommand[index] == '(') && !isInString(inCommand.substr(0, index))) ++bracketCnt;
					if ((inCommand[index] == ')') && !isInString(inCommand.substr(0, index))) --bracketCnt;
				}
				funcProt.endPos = index;
				funcProt.params = inCommand.substr(end + 1, index - end+1);
				listFunc.push_back(funcProt);
			}
		}
		++index;
	}
	//WriteLogDebug("List func size - " + to_string(listFunc.size()));
	if (listFunc.size() == 0)
	{
		std::string note;
		genUml.note(inCurrentClass, inCommand, note);
		outResult += note;
	}
	else if (listFunc.size() == 1)
	{
		if (listFunc[0].startPos > 0)
		{
			//outResult += inCommand.substr(0, listFunc[0].startPos); 
			std::string str;
			std::string depend;
			GetDependency(listFunc[0].funcName, depend);
			std::string commandStr = inCommand;
			genUml.funcCall(inCurrentClass, depend, commandStr, str);
			outResult += str;
			commandStr = "";
			genUml.funcReturn(inCurrentClass, depend, commandStr, str);
			outResult += str;
		}
	}
	else
	{
		int varID = 0;
		for (auto& funcProt : listFunc)
		{
			++varID;
			std::string str;
			std::string depend;
			GetDependency(funcProt.funcName, depend);
			std::string funcCall = "Var_" + std::to_string(varID) + " = " + funcProt.funcName + funcProt.params;
			genUml.funcCall(inCurrentClass, depend, funcCall, str);
			outResult += str;
			std::string funcReturn = "";
			genUml.funcReturn(inCurrentClass, depend, funcReturn, str);
			outResult += str;
		}
		int startPos = 0;
		string src;
		for (int idx = 0; idx < varID; idx++)
		{
			if (startPos < listFunc[idx].startPos)
			{
				src += inCommand.substr(startPos, listFunc[idx].startPos - startPos);
			}
			src += "Var_" + std::to_string(idx + 1);
			startPos = listFunc[idx].endPos +1;
		}
		if (startPos < inCommand.size())
		{
			src += inCommand.substr(startPos, inCommand.size() - startPos);
		}
		string outSrc;
		genUml.note(inCurrentClass, src, outSrc);
		outResult += outSrc;
	}
}

bool ParseSource::checkList(char inChar)
{
	if (inChar == '+') return true;
	if (inChar == '-') return true;
	if (inChar == '*') return true;
	if (inChar == '/') return true;
	if (inChar == ' ') return true;
	return false;
}

bool ParseSource::isFuncNameExist(std::string& inFuncName)
{
	for (const auto& depend : listDepend)
	{
		for (const auto& funcName : depend.funcsName)
		{
			if (funcName == inFuncName) return true;
		}
	}
	return false;
}

void ParseSource::GetDependency(std::string& inFuncName, std::string& outDepend)
{
	for (const auto& depend : listDepend)
	{
		for (const auto& funcName : depend.funcsName)
		{
			if (funcName == inFuncName)
			{
				outDepend = depend.fileName;
				return;
			}
		}
	}
}

bool ParseSource::isFuncNameValid(const std::string& inFuncName)
{
	if (inFuncName == "") return false;
	if (inFuncName == " ") return false;
	if (inFuncName.find(" ") != string::npos) return false;
	return true;
}

bool ParseSource::isInString(const std::string inContent)
{
	int quotaCnt = 0;
	for (int idx = 0; idx < inContent.length(); idx++)
	{
		if ((inContent[idx] == '"') && (idx == 0))
		{
			++quotaCnt;
			continue;
		}
		if ((inContent[idx] == '"') && (inContent[idx - 1] != '\\')) ++quotaCnt;
	}
	if (quotaCnt % 2 != 0) return true;
	return false;
}

bool ParseSource::checkValidFuncDef(std::string& inFuncDef)
{
	const static char blankList[] = {' ', '\n', '\t', '\r'};
	if (inFuncDef == "") return false;
	if (inFuncDef == " ") return false;
	for (int idx = 0; idx < inFuncDef.length(); idx++)
	{
		if (isalpha(inFuncDef[idx])) return true;
		for (auto& item : blankList)
		{
			if (inFuncDef[idx] != item) return true;
		}
	}
	return false;
}

void ParseSource::ContentFilter(std::string& inoutContent)
{
	static const string listPre[] = { "#if ", "#ifdef ", "#ifndef ", "#else", "#endif" };
	static const string listReplace[] = { "if (", "if (", "if (", "} else {", "} " };
	// remove comment, preprocess
	istringstream tempContent(inoutContent);
	inoutContent = "";
	string lineContent;
	bool isBlockCmt = false;
	while (getline(tempContent, lineContent))
	{
		int index = 0;
		while (true)
		{
			index = lineContent.find("\\");
			if ((lineContent.length()) && (index == (lineContent.length() - 1)))
			{
				lineContent.replace(lineContent.length() - 1, 1, " ");
				string nextLine;
				if (getline(tempContent, nextLine))
				{
					lineContent += nextLine;
				}
			}
			else
			{
				break;
			}
		}
		//WriteLogDebug("----" + lineContent + "---");
		if ((lineContent == "\n") && !isBlockCmt) continue;
		if ((lineContent == "") && !isBlockCmt) continue;
		if ((lineContent.find(DEF_STR) != string::npos) && !isBlockCmt) continue;
		if ((lineContent.find(UNDEF_STR) != string::npos) && !isBlockCmt) continue;
		if ((lineContent.find(INC_STR) != string::npos) && !isBlockCmt) continue;
		//if ((lineContent.find(INLINE_STR) != string::npos) && !isBlockCmt) continue;
		if ((lineContent.find(NAMESPACE_STR) != string::npos) && !isBlockCmt) continue;
		int idx = lineContent.find(LINE_CMT_STR);
		if ((idx != string::npos) && !isBlockCmt)
		{
			inoutContent += (lineContent.substr(0, idx) + "\n");
			continue;
		}

		idx = lineContent.find(BEGIN_BLK_CMT_STR);
		if ((idx != string::npos) && !isBlockCmt)
		{
			inoutContent += (lineContent.substr(0, idx) + "\n");
			isBlockCmt = true;
			//continue;
			lineContent = lineContent.substr(idx + 1);
		}

		idx = lineContent.find(END_BLK_CMT_STR);
		if ((idx != string::npos) && isBlockCmt)
		{
			inoutContent += (lineContent.substr(idx + 2) + "\n");
			isBlockCmt = false;
			continue;
		}
		if (!isBlockCmt)
		{
			for (int idx = 0; idx < _countof(listPre); idx++)
			{
				int index = lineContent.find(listPre[idx]);
				if (index != string::npos)
				{
					if (idx <= 2)
					{
						lineContent = listReplace[idx] + lineContent + ") {";
					}
					else
					{
						lineContent.replace(index, listPre[idx].length(), listReplace[idx]);
					}
				}
			}
			inoutContent += (lineContent + '\n');
		}
	}
	//WriteLogDebug("File content after remove comment, preprocessor: ");
	//WriteLogDebug(inoutContent);

	// remove tab
	int index = 0;
	while (index != std::string::npos)
	{
		index = inoutContent.find('\t');
		if (index != std::string::npos)
		{
			inoutContent.replace(index, 1, " ");
		}
	}
	//WriteLogDebug("File content after remove tab: ");
	//WriteLogDebug(inoutContent);

	// remove double spaces
	index = 0;
	while (index != std::string::npos)
	{
		index = inoutContent.find("  ");
		if (index != std::string::npos)
		{
			inoutContent.replace(index, 2, " ");
		}
	}
	logFile->WriteLogToFile("File content after remove double spaces: ");
	logFile->WriteLogToFile(inoutContent);

	// remove blank line
	index = 0;
	while (index != std::string::npos)
	{
		index = inoutContent.find("\n\n");
		if (index != std::string::npos)
		{
			inoutContent.erase(index, 1);
		}
	}
	//WriteLogDebug("File content after remove blank line: ");
	//WriteLogDebug(inoutContent);
}

void ParseSource::GetFuncDef(const std::string& inFileName,const std::string& inFilecontent)
{
	//WriteLogDebug("Get function def - file name: " + inFileName);
	unsigned long long start = 0;
	unsigned long long index = 0;
	string funcNameStr = ERR_FUNCNAME;
	while (true)
	{
		//WriteLogDebug("index = " + to_string(index));
		//WriteLogDebug("start = " + to_string(start));
		//WriteLogDebug("filecontent len = " + to_string(inFilecontent.length()));
		if (start >= inFilecontent.length()) break;
		if (index >= inFilecontent.length()) break;

		//WriteLogDebug(inFilecontent[index] + "");
		if ((inFilecontent[index] == '{') && !isInString(inFilecontent.substr(0, index)))
		{
			
			// Get func name
			string funcName = inFilecontent.substr(start, index - start);
			//WriteLogDebug(to_string((int)(' ')));
			//WriteLogDebug("func: " + funcName);
			int idx = funcName.length() - 1;
			while (idx >= 0)
			{
				//WriteLogDebug(to_string((int)funcName[idx]));
				if ((funcName[idx] == '(') && !isInString(inFileName.substr(0, idx)))
				{
					funcName = funcName.substr(0, idx);
					Filter(funcName);
					int begin = funcName.length() - 1;
					for (; begin >= 0; begin--)
					{
						if (funcName[begin] == ' ') break;
						if (checkList(funcName[begin])) break;
					}
					funcNameStr = funcName.substr(begin + 1);
					if ((begin > 0) && isFuncNameValid(funcNameStr))
					{
						AddDepend(inFileName, funcNameStr);
					}
					else
					{
						cout << "Parse function error: Func name is empty" << endl;
					}
				}
				--idx;
			}
			//WriteLogDebug("func name: " + funcNameStr);
			if (funcNameStr != ERR_FUNCNAME)
			{
				// Get func define
				++index;
				start = index;
				int curlyBracketCnt = 1;
				while ((curlyBracketCnt != 0) && (index < inFilecontent.length()))
				{
					if ((inFilecontent[index] == '{') && !isInString(inFilecontent.substr(0, index))) ++curlyBracketCnt;
					if ((inFilecontent[index] == '}') && !isInString(inFilecontent.substr(0, index))) --curlyBracketCnt;
					++index;
				}
				if (curlyBracketCnt != 0)
				{
					cout << "Parse file error: no find the end of function " << funcNameStr << endl;
				}
				else {
					FuncDefine funcDef;
					funcDef.funcName = funcNameStr;
					funcDef.funcDef = inFilecontent.substr(start, index - start -1);

					if (checkValidFuncDef(funcDef.funcDef))
					{
						listFuncDef.push_back(funcDef);
						funcNameStr = ERR_FUNCNAME;
					}
					//WriteLogDebug("Func def: " + funcDef.funcDef);
				}
			}
			start = index + 1;
		}
		++index;
	}
}

void ParseSource::Filter(std::string& inoutContent)
{
	// remove tab
	int index = -1;
	while (index != string::npos)
	{
		index = inoutContent.find('\t');
		if (index != string::npos)
		{
			inoutContent.replace(index, 1, " ");
		}
	}

	// remove \n
	index = -1;
	while (index != string::npos)
	{
		index = inoutContent.find('\n');
		if (index != string::npos)
		{
			inoutContent.replace(index, 1, " ");
		}
	}

	// remove spaces in begin of line
	while (inoutContent[0] == ' ') inoutContent.erase(0, 1);

	// remove spaces in end of line
	while (inoutContent.length() > 0)
	{
		if (inoutContent[inoutContent.length() - 1] == ' ')
		{
			inoutContent.erase(inoutContent.length() - 1, 1);
		}
		else
		{
			break;
		}
	}
}

void ParseSource::AddDepend(const std::string inFileName, const std::string inFuncName)
{
	if (!isFuncNameValid(inFuncName)) return;
	bool existFileName = false;
	for (auto & depend: listDepend)
	{
		if (depend.fileName == inFileName)
		{
			existFileName = true;
			break;
		}
	}
	if (!existFileName)
	{
		Depend def;
		def.fileName = inFileName;
		listDepend.push_back(def);
	}
	for (auto& depend : listDepend)
	{
		if (depend.fileName == inFileName)
		{
			bool existFuncName = false;
			for (auto& funcName : depend.funcsName)
			{
				if (funcName == inFuncName)
				{
					existFuncName = true;
					break;
				}
			}
			if (!existFuncName)
			{
				depend.funcsName.push_back(inFuncName);
			}
		}
	}
}

void ParseSource::GetListFile()
{
	//WriteLogDebug("Get list file");
	struct stat sb;
	if (stat(sourceDir.c_str(), &sb) == 0)
	{
		cout << "List files in the source dir (" << sourceDir << "): \n";
		for (const auto& index : std::filesystem::recursive_directory_iterator(sourceDir))
		{
			if (!std::filesystem::is_directory(index.path()))
			{
				string filePath = index.path().string();
				if (filePath.find(FILE_EXTENSION) != string::npos)
				{
					filePaths.push_back(filePath);
					std::cout << filePath << "\n";
				}
			}
		}
	}
	else
	{
		cout << "Source dir is not exist (" << sourceDir << ")";
	}
}

int ParseSource::GetFileNum()
{
	return filePaths.size();
}

void ParseSource::GetFileName(const std::string& inFilePath, std::string& outFileName)
{
	outFileName = inFilePath.substr(inFilePath.find_last_of("/\\") + 1);
	outFileName = outFileName.substr(0, outFileName.find_first_of('.'));
}
