#include "CLog.h"

CLog::CLog()
{
	::CreateDirectoryA("Log", NULL);				//�۾����� �ش� ������Ʈ
	SetPathFile();
}

void CLog::SetPathFile()
{
	char caPath[MAX_PATH] = { 0, };
	GetCurrentDirectoryA(MAX_PATH, caPath);			//�۾����� �ش� ������Ʈ

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s", caPath, "\\log\\");	//�װ��� ������ log����
	sprintf_s(m_caFileName, MAX_FILENAME_LEN, "%s", SAVEFILENAME);
}

void CLog::Log(const char * loglevel, const char* logmsg)
{
	SYSTEMTIME t;
	GetLocalTime(&t);

	FILE* pFile = NULL;
	char szFileName[MAX_PATHNAME_LEN] = { 0, };		//������ ���� ��� + �̸�
	sprintf_s(szFileName, "%s%s_%04d%02d%02d.log", m_caPathName, m_caFileName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pFile, szFileName, "at");		//append text ���
	if (pFile)
	{
		fprintf_s(pFile, "[%02d:%02d:%02d.%03d][%s] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, loglevel, logmsg);
									//�α� ���� �ۼ�
		fclose(pFile);
	}
	else
	{
		
	}
}
