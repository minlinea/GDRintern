#pragma once

#include <string>
#include <Windows.h>

#define MAX_PATHNAME_LEN 1024
#define MAX_FILENAME_LEN 256
#define MAX_MESSAGE_LEN 256

#define SAVEFILENAME "test"

class CLog
{
public:
	CLog();
	~CLog();

	void SetPathFile();
	void Log(const char* loglevel, const char * logmsg);
private:
	char m_caPathName[MAX_PATHNAME_LEN];
};

