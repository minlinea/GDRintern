#include "CLog.h"

CLog::CLog()
{
	::CreateDirectoryA("Log", NULL);				//작업중인 해당 프로젝트
	SetPathFile();
	Log("START", "==============================================");
}

CLog::~CLog()
{
	Log("END", "==============================================");
}

void CLog::SetPathFile()
{
	char caPath[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, caPath);			//작업중인 해당 프로젝트

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s%s", caPath, "\\log\\", SAVEFILENAME);	//그곳에 생성된 log폴더
}

void CLog::Log(const char * logtype, const char* logmsg)
{
	SYSTEMTIME t;
	GetLocalTime(&t);

	FILE* pfile = NULL;
	char filename[MAX_PATHNAME_LEN] = { 0, };		//생성할 파일 경로 + 이름
	sprintf_s(filename, "%s_%04d%02d%02d.log", m_caPathName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pfile, filename, "at");		//append text 모드
	if (pfile)
	{
		fprintf_s(pfile, "[%02d:%02d:%02d.%03d][%s]\t%s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, logtype, logmsg);
									//로그 내용 작성
		fclose(pfile);
	}
	else
	{
		
	}
}