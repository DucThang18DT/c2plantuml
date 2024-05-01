#include "GenerateUML.h"
#define FOR_STR	"for "
#define WHILE_STR	"while "
#define IF_STR	"if "
#define ELSE_STR	"else"
#define ELSE_IF_STR	"else if "
#define SWITCH_STR	"switch"
#define CASE_STR		"case "
#define DEFAULT_STR		"default"
#define OPEN_NOTE	"NOTE OVER OF "
#define END_NOTE	"END NOTE\n"
#define STATEMENT_END	"END"
#define ELSE_KEY		"ELSE "
#define DOUBLE_NOTE		END_NOTE+OPEN_NOTE
#define INCORRECT_IF_ELSE (STATEMENT_END + "\n" + ELSE_KEY)

const static std::string st_StatementStr[] = {FOR_STR, WHILE_STR, ELSE_IF_STR, IF_STR, ELSE_STR, SWITCH_STR, CASE_STR, DEFAULT_STR};
const static std::string st_StatementUMLStr[] = { "LOOP ", "LOOP ", ELSE_KEY, "ALT ", ELSE_KEY, "", "ALT ", ELSE_KEY};

GenerateUML::GenerateUML()
{
}

GenerateUML::~GenerateUML()
{
}

void GenerateUML::Statement(const std::string& inStatement, bool inIsFirstCase, std::string& outResult, bool& outSwitchCase, std::string& inoutSwitchVar)
{
	outSwitchCase = false;
	//static std::string inoutSwitchVar;
	for (int id = 0; id < _countof(st_StatementStr); id++)
	{
		int index = inStatement.find(st_StatementStr[id]);
		if (index != std::string::npos)
		{
			std::string condition = inStatement.substr(index + st_StatementStr[id].length()); 
			if (st_StatementStr[id] == CASE_STR)
			{
				if (!inIsFirstCase)
				{
					outResult = (ELSE_KEY + inoutSwitchVar + " == " + condition);
				}
				else
				{
					outResult = st_StatementUMLStr[id] + inoutSwitchVar + " == " + condition;
				}
			}
			else if (st_StatementStr[id] == DEFAULT_STR)
			{
				if (inIsFirstCase)
				{
					outResult = "ALT " + condition;
				}
				else
				{
					outResult = st_StatementUMLStr[id] + condition;
				}
			}
			else if (st_StatementStr[id] == SWITCH_STR)
			{
				outSwitchCase = true;
				int cmpVarIdx = 0;
				while (cmpVarIdx < condition.length())
				{
					if (condition[cmpVarIdx] == '(')
					{
						int bracketCnt = 1;
						int endIdx = cmpVarIdx + 1;
						while (bracketCnt && (endIdx < condition.length()))
						{
							if (condition[endIdx] == '(') ++bracketCnt;
							if (condition[endIdx] == ')') --bracketCnt;
							++endIdx;
 						}
						if (bracketCnt)
						{
							std::cout << "parse switch-case failed \n";
						}
						else
						{
							inoutSwitchVar = condition.substr(cmpVarIdx +1, endIdx - cmpVarIdx - 2);
						}
					}
					++cmpVarIdx;
				}
			}
			else
			{
				outResult = st_StatementUMLStr[id] + condition;
			}
			break;
		}
	}
	int index = 0;
	while (true)
	{
		index = outResult.find('\n');
		if (index != std::string::npos)
		{
			outResult.replace(index, 1, " ");
		}
		else
		{
			break;
		}
	}
}

void GenerateUML::OptimizeUmlSource(std::string& inoutSource)
{
	int index = 0;
	while (true)
	{
		index = inoutSource.find("\n\n");
		if (index != std::string::npos)
		{
			inoutSource.erase(index+1, 1);
		}
		else
		{
			break;
		}
	}

	const std::string doubleNote = std::string(END_NOTE)+ std::string(OPEN_NOTE);
	index = -1;
	while (true)
	{
		index = inoutSource.find(doubleNote);
		if (index != std::string::npos)
		{
			/*
			* remove "END NOTE\nNOTE OVER OF xxxx\n"
			* endOfLine is start of the line "NOTE OVER OF xxxx\n"
			*/
			int endOfLine = index + std::string(END_NOTE).length(); // 
			while (endOfLine < inoutSource.length())
			{
				if (inoutSource[endOfLine] == '\n') break;
				++endOfLine;
			}
			inoutSource.erase(index, endOfLine - index +1); 
		}
		else
		{
			break;
		}
	}

	const std::string incorrectIfElse = std::string(STATEMENT_END) + "\n" + std::string(ELSE_KEY);
	index = -1;
	while (true)
	{
		index = inoutSource.find(incorrectIfElse);
		if (index != std::string::npos)
		{
			inoutSource.erase(index, 4); // remove "END\n"
		}
		else
		{
			break;
		}
	}
}

void GenerateUML::funcCall(const std::string& inFrom, const std::string& inDes, std::string& inFuncCall, std::string& outResult)
{
	int index = 0;
	while (true)
	{
		index = inFuncCall.find('\n');
		if (index != std::string::npos)
		{
			inFuncCall.replace(index, 1, " ");
		}
		else
		{
			break;
		}
	}
	outResult = inFrom + "->" + inDes + "++: " + inFuncCall + "\n";
}

void GenerateUML::funcReturn(const std::string& inDes, const std::string& inFrom, std::string& inReturn, std::string& outResult)
{
	int index = 0;
	while (true)
	{
		index = inReturn.find('\n');
		if (index != std::string::npos)
		{
			inReturn.replace(index, 1, " ");
		}
		else
		{
			break;
		}
	}
	outResult = inDes + "<--" + inFrom + "--: " + inReturn + "\n";
}

void GenerateUML::note(const std::string& inClass, const std::string& inContent, std::string& outResult)
{
	outResult = OPEN_NOTE + inClass + "\n" + inContent + "\n" + END_NOTE;
}
