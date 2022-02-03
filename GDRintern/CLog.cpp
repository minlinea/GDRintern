#include "CLog.h"

CLog::CLog()
{
	::CreateDirectoryA("Log", NULL);				//작업중인 해당 프로젝트
	SetPathFile();
}

void CLog::SetPathFile()
{
	char caPath[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, caPath);			//작업중인 해당 프로젝트

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s", caPath, "\\log\\");	//그곳에 생성된 log폴더
	sprintf_s(m_caFileName, MAX_FILENAME_LEN, "%s", SAVEFILENAME);
}

void CLog::Log(const char * loglevel, const char* logmsg)
{
	SYSTEMTIME t;
	GetLocalTime(&t);

	FILE* pFile = NULL;
	char szFileName[MAX_PATHNAME_LEN] = { 0, };		//생성할 파일 경로 + 이름
	sprintf_s(szFileName, "%s%s_%04d%02d%02d.log", m_caPathName, m_caFileName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pFile, szFileName, "at");		//append text 모드
	if (pFile)
	{
		fprintf_s(pFile, "[%02d:%02d:%02d.%03d][%s] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, loglevel, logmsg);
									//로그 내용 작성
		fclose(pFile);
	}
	else
	{
		
	}
}
