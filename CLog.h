#pragma once

#include <string>
#ifdef __cplusplus
#include "windows.h"
#endif
#ifndef __cplusplus
#include "window.h"
#endif

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
	void Log(const char* logtype, const char * logmsg);
private:
	char m_caPathName[MAX_PATHNAME_LEN];
};

