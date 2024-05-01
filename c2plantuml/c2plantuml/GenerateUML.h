#pragma once
#include <iostream>

enum CommandType {
	FuncCall,
	Calc
};
class GenerateUML
{
public: 
	GenerateUML();
	~GenerateUML();

	// Gen Uml source for function call
	void funcCall(const std::string& inFrom, const std::string& inDes, std::string& inFuncCall, std::string& outResult);

	// gen uml source for a return
	void funcReturn(const std::string& inDes, const std::string& inFrom, std::string& inReturn, std::string& outResult);

	// gen uml source for a note
	void note(const std::string& inClass, const std::string& inContent, std::string& outResult);

	// gen uml source for a statement: if, else, for, while
	void Statement(const std::string& inStatement, bool inIsFirstCase, std::string& outResult, bool& outSwitchCase, std::string& inoutSwitchVar);

	void OptimizeUmlSource(std::string& inoutSource);
private:
	
};

