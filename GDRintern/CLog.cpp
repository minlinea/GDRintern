#include "CLog.h"

CLog::CLog()
{
	char caPath[MAX_PATH] = { 0, };
	GetModuleFileNameA(NULL, caPath, MAX_PATH);

	char szDrive[_MAX_DRIVE] = { 0, };
	char szDir[MAX_PATHNAME_LEN] = { 0, };
	char szName[_MAX_FNAME] = { 0, };
	char szExt[_MAX_EXT] = { 0, };
	_splitpath_s(caPath, szDrive, _MAX_DRIVE, szDir, MAX_PATHNAME_LEN, szName, _MAX_FNAME, szExt, _MAX_EXT);

	sprintf_s(m_caPathName, MAX_PATHNAME_LEN, "%s%s", szDrive, szDir);
	sprintf_s(m_caFileName, MAX_FILENAME_LEN, "%s", "TEST.txt");
}