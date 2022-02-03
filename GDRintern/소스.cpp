#include <iostream>
#include <Windows.h>
#include <atlconv.h>
#include <fstream>
#include "conio.h"
#include "windows.h"
#include "packet.h"
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
		if(1 == _kbhit())	//1�� true�ε� true�� �ϸ� ���
		{
			c = _getch();

			if (c == 'q')
			{
				char path[MAX_PATH] = { 0, };		
				GetModuleFileNameA(NULL, path, MAX_PATH);
				string str(path);
				cout << str << endl;
				//�����ڵ� ��� �� wchar_t�� GetModuleFileNameW ���
			}
			else if (c == 'w')
			{
				char path[MAX_PATH] = { 0, };
				GetModuleFileNameA(NULL, path, MAX_PATH);
				string str(path);
				int t = str.rfind('\\');		//������ \�ִ� ������ ã�Ƴ� ��ġ
				string str2 = str.substr(t, str.size() - t);	//�ش� ������ �̸��� �� (\+[���� ���α׷� ����])
				str.erase(str.rfind('\\'));						//�ش� �κи� ������ ���� ����
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

				string drive(szDrive);	//Drive�� (C, D, E �̷���)
				string dir(szDir);		//Drive�� ������ ���(\~)
				string str(pathname);	//sprintf_s�� ������ ���� ���
				string str2(filename);	//sprintf_s�� ������ ���� �̸�
				string t(szName);		//�ش� ���α׷��� �̸�
				string t2(szExt);		//�ش� ���α׷��� ���� Ÿ��
				string t3(test);		//sprintf_s�� ������ �̸� + ���� Ÿ��

				cout << drive << endl;	//C:
				cout << dir << endl;	//\Users\line0413\Desktop\���Ͻ�\GDR����������Ʈ\GDRintern\Debug\//
				cout << str << endl;	//C:\Users\line0413\Desktop\���Ͻ�\GDR����������Ʈ\GDRintern\Debug\//
				cout << str2 << endl;	//TEST.txt
				cout << t << endl;		//GDRintern
				cout << t2 << endl;		//.exe
				cout << t3 << endl;		//GDRintern.exe					
			}
			else if (c == 'r')
			{
				SYSTEMTIME t;
				GetLocalTime(&t);
			}
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}