#include <iostream>
#include <Windows.h>
#include <atlconv.h>
#include <fstream>
#include "conio.h"
#include "windows.h"
#include "packet.h"
#include <ctime>
#include "CLog.h"
using namespace std;

void CALLBACK MyTimerProc(HWND hWnd, UINT nMsg, UINT_PTR nIDEvent, DWORD dwTime)
{
	cout << "timerproccccc" << endl;
}

int main()
{
	time_t t1;
	time_t t2;
	CLog clog;
	char c;
	time(&t1);
	while(true)
	{
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
				time(&t2);

				if (t2 - t1 >= 2)
				{
					cout << "2sec" << endl;
					time(&t1);
				}
			}
			
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}