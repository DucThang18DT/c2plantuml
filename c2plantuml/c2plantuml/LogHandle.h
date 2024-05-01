#pragma once
#include <iostream>

#define DEBUG

static void WriteLogDebug(std::string inLog)
{
#ifdef DEBUG
	std::cout << inLog << "\n";
#endif // DEBUG
}

class LogHandle
{
public:
	// Singleton pattern
	static LogHandle* GetInstance(std::string& inLogDir);
	~LogHandle();

	// Write to log file
	void WriteLogToFile(std::string inLog);
private:
	LogHandle(std::string& inLogDir);
	static LogHandle* instance;
	std::string logPath;
};

