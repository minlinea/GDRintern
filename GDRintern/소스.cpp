#include <iostream>
#include <Windows.h>
#include <atlconv.h>
#include <fstream>
#include "conio.h"
#include "windows.h"
#include "packet.h"
#include "CLog.h"
using namespace std;

int main()
{
	CLog clog;
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
				clog.Log("error", "������������������");
				cout << "write" << endl;

			}
			
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}