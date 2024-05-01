#include "LogHandle.h"
#include <fstream>
#include <filesystem>

LogHandle*  LogHandle::instance = nullptr;

LogHandle::LogHandle(std::string& inLogDir)
{
	if (std::filesystem::exists(inLogDir))
	{
		std::filesystem::create_directories(inLogDir);
	}
	logPath = inLogDir + "\\Log.txt";
}

LogHandle* LogHandle::GetInstance(std::string& inLogDir)
{
	if (instance == nullptr)
	{
		instance = new LogHandle(inLogDir);
	}
	return instance;
}

LogHandle::~LogHandle()
{
	//delete instance;
}

void LogHandle::WriteLogToFile(std::string inLog)
{
#ifdef DEBUG
	std::ofstream outLogFile(logPath, std::ios::app | std::ios::out);
	outLogFile << "\n----------LOG CONTENT----------\n" << inLog << "\n----------END LOG CONTENT----------\n";
	outLogFile.close();
#endif // DEBUG
}
