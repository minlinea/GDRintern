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

	FILE* pFile = NULL;
	char caFileName[MAX_PATHNAME_LEN] = { 0, };		//������ ���� ��� + �̸�
	sprintf_s(caFileName, "%s_%04d%02d%02d.log", m_caPathName, t.wYear, t.wMonth, t.wDay);

	fopen_s(&pFile, caFileName, "at");		//append text ���
	if (pFile)
	{
		fprintf_s(pFile, "[%02d:%02d:%02d.%03d][%s]\t%s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, logtype, logmsg);
									//�α� ���� �ۼ�
		fclose(pFile);
	}
	else
	{
		
	}
}

void CLog::MakeMsg(const char* logmsg, ...)
{
	va_list args;
	va_start(args, logmsg);

	char msg[MAX_MESSAGE_LEN] = { 0, };
	vsnprintf_s(msg, sizeof(msg), MAX_MESSAGE_LEN, logmsg, args);

	va_end(args);

	this->Log("INFO", msg);
}