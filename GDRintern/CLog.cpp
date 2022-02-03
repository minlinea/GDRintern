#include "CLog.h"

CLog::CLog()
{
	SetPathFile();
}

void CLog::SetPathFile()
{
	char caPath[MAX_PATH] = { 0, };
	GetModuleFileNameA(NULL, caPath, MAX_PATH);

	char szDrive[_MAX_DRIVE] = { 0, };
	char szDir[MAX_PATHNAME_LEN] = { 0, };
	char szName[_MAX_FNAME] = { 0, };
	char szExt[_MAX_EXT] = { 0, };
	_splitpath_s(caPath, szDrive, _MAX_DRIVE, szDir, MAX_PATHNAME_LEN, szName, _MAX_FNAME, szExt, _MAX_EXT);
	//szName(실행프로그램 이름) + szExt(실행프로그램 타입) 

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s", szDrive, szDir);
	sprintf_s(m_caFileName, MAX_FILENAME_LEN, "%s", SAVEFILENAME);
}

void CLog::Log(const char * loglevel, const char* logmsg)
{
	SYSTEMTIME t;
	GetLocalTime(&t);

	FILE* pFile = NULL;
	char szFileName[MAX_PATHNAME_LEN] = { 0, };
	sprintf_s(szFileName, "%s%s_%04d%02d%02d.log", m_caPathName, m_caFileName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pFile, szFileName, "at");
	if (pFile)
	{
		fprintf_s(pFile, "[%02d:%02d:%02d.%03d][%s] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, loglevel, logmsg);	//마지막 test를 인자로 받아 사용

		fclose(pFile);
	}
	else
	{
		
	}
}
