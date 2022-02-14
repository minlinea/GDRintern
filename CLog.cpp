#include "CLog.h"

CLog::CLog()
{
	::CreateDirectoryA("Log", NULL);				//�۾����� �ش� ������Ʈ
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
	GetCurrentDirectoryA(MAX_PATH, caPath);			//�۾����� �ش� ������Ʈ

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s%s", caPath, "\\log\\", SAVEFILENAME);	//�װ��� ������ log����
}

void CLog::Log(const char * logtype, const char* logmsg)
{
	SYSTEMTIME t;
	GetLocalTime(&t);

	FILE* pfile = NULL;
	char filename[MAX_PATHNAME_LEN] = { 0, };		//������ ���� ��� + �̸�
	sprintf_s(filename, "%s_%04d%02d%02d.log", m_caPathName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pfile, filename, "at");		//append text ���
	if (pfile)
	{
		fprintf_s(pfile, "[%02d:%02d:%02d.%03d][%s]\t%s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, logtype, logmsg);
									//�α� ���� �ۼ�
		fclose(pfile);
	}
	else
	{
		
	}
}