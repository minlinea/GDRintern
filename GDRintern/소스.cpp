#include <iostream>
#include <Windows.h>
#include <atlconv.h>
#include <fstream>
#include "conio.h"
#include "windows.h"
#include "packet.h"
#include "CLog.h"
using namespace std;

#define MAX_PATHNAME_LEN 1024
#define MAX_FILENAME_LEN 256
#define MAX_MESSAGE_LEN 256

int main()
{
	char c;
	while(true)
	{
		ShotData sd{0,1,2,3,4,5,6};
		if(1 == _kbhit())	//1이 true인데 true라 하면 경고냄
		{
			c = _getch();

			if (c == 'q')
			{
				char path[MAX_PATH] = { 0, };		
				GetModuleFileNameA(NULL, path, MAX_PATH);
				string str(path);
				cout << str << endl;
				//유니코드 사용 시 wchar_t에 GetModuleFileNameW 사용
			}
			else if (c == 'w')
			{
				char path[MAX_PATH] = { 0, };
				GetModuleFileNameA(NULL, path, MAX_PATH);
				string str(path);
				int t = str.rfind('\\');		//마지막 \있는 지점을 찾아낸 위치
				string str2 = str.substr(t, str.size() - t);	//해당 파일의 이름을 얻어냄 (\+[실행 프로그램 파일])
				str.erase(str.rfind('\\'));						//해당 부분만 지워낼 수도 있음
				cout << str2 << " " << str << endl;
			}
			else if (c == 'e')
			{
				char path[MAX_PATH] = { 0, };
				char pathname[MAX_PATH] = { 0, };
				char filename[MAX_PATH] = { 0, };
				char test[MAX_PATH] = { 0, };
				GetModuleFileNameA(NULL, path, MAX_PATH);

				char szDrive[_MAX_DRIVE] = { 0, };
				char szDir[MAX_PATHNAME_LEN] = { 0, };
				char szName[_MAX_FNAME] = { 0, };
				char szExt[_MAX_EXT] = { 0, };
				_splitpath_s(path, szDrive, _MAX_DRIVE, szDir, MAX_PATHNAME_LEN, szName, _MAX_FNAME, szExt, _MAX_EXT);

				sprintf_s(pathname, MAX_PATHNAME_LEN, "%s%s", szDrive, szDir);
				sprintf_s(filename, MAX_FILENAME_LEN, "%s", "TEST.txt");
				sprintf_s(test, MAX_FILENAME_LEN, "%s%s", szName, szExt);

				string drive(szDrive);	//Drive만 (C, D, E 이런거)
				string dir(szDir);		//Drive를 제외한 경로(\~)
				string str(pathname);	//sprintf_s로 조합한 파일 경로
				string str2(filename);	//sprintf_s로 설정한 파일 이름
				string t(szName);		//해당 프로그램의 이름
				string t2(szExt);		//해당 프로그램의 실행 타입
				string t3(test);		//sprintf_s로 조합한 이름 + 실행 타입

				cout << drive << endl;	//C:
				cout << dir << endl;	//\Users\line0413\Desktop\인턴십\GDR인턴프로젝트\GDRintern\Debug\//
				cout << str << endl;	//C:\Users\line0413\Desktop\인턴십\GDR인턴프로젝트\GDRintern\Debug\//
				cout << str2 << endl;	//TEST.txt
				cout << t << endl;		//GDRintern
				cout << t2 << endl;		//.exe
				cout << t3 << endl;		//GDRintern.exe					
			}
			else if (c == 'r')
			{
				SYSTEMTIME t;
				GetLocalTime(&t);
				int date[6] = { t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond};
				cout << "[" << date[0] << "." << date[1] << "." << date[2] << " " << date[3] << ":"
					<< date[4] << ":" << date[5] << "] : " << endl;
			}
			else if (c == 't')
			{
				char path[MAX_PATH] = { 0, };
				char pathname[MAX_PATH] = { 0, };
				char filename[MAX_PATH] = { 0, };
				GetModuleFileNameA(NULL, path, MAX_PATH);

				char szDrive[_MAX_DRIVE] = { 0, };
				char szDir[MAX_PATHNAME_LEN] = { 0, };
				char szName[_MAX_FNAME] = { 0, };
				char szExt[_MAX_EXT] = { 0, };
				_splitpath_s(path, szDrive, _MAX_DRIVE, szDir, MAX_PATHNAME_LEN, szName, _MAX_FNAME, szExt, _MAX_EXT);

				sprintf_s(pathname, MAX_PATHNAME_LEN, "%s%s", szDrive, szDir);
				sprintf_s(filename, MAX_FILENAME_LEN, "%s", "TEST");

				string str(pathname);	//sprintf_s로 조합한 파일 경로
				string str2(filename);	//sprintf_s로 설정한 파일 이름


				SYSTEMTIME t;
				GetLocalTime(&t);

				FILE* pFile = NULL;
				char szFileName[MAX_PATHNAME_LEN] = { 0, };
				sprintf_s(szFileName, "%s%s_%04d%02d%02d.log", pathname, filename, t.wYear, t.wMonth, t.wDay);

				fopen_s(&pFile, szFileName, "at");
				if (pFile)
				{
					fprintf_s(pFile, "[%02d:%02d:%02d.%03d][%s] %s\n", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds, "Warn", "test");	//마지막 test를 인자로 받아 사용

					cout << "pFile open\n";
					fclose(pFile);
				}
				else
				{
					cout << "pFile not open\n";
				}

				cout << "pFile\n";
			}
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}