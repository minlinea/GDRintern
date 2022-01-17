#include <iostream>
#include <thread>
#include "windows.h"
using namespace std;

DWORD WINAPI SendThread(LPVOID socket)//������ ���� �Ѱ��ֱ�
{
	cout << "SendThread\n";

	return NULL;
}

DWORD WINAPI RecvThread(LPVOID socket)
{
	cout << "RecvThread\n";
	while (true)
	{

	}
	return NULL;
}



int main()
{
	DWORD dwSendThreadID, dwRecvThreadID;

	cout << "MainThread ����\n";

	HANDLE hSend = CreateThread(0, 0, SendThread, NULL, 0, &dwSendThreadID);

	DWORD retvalSend = WaitForSingleObject(hSend, INFINITE);
	if (retvalSend == WAIT_OBJECT_0)
	{
		cout << "Send ������ ����\n";
	}
	else if (retvalSend == WAIT_TIMEOUT)	//�������̶� �Ȱɸ�
	{
		cout << "Ÿ�Ӿƿ�\n";
	}
	else
	{
		cout << "���� �߻�\n";
	}
	HANDLE hRecv = CreateThread(0, 0, RecvThread, NULL, 0, &dwRecvThreadID);

	DWORD retvalRecv = WaitForSingleObject(hRecv, INFINITE);
	if (retvalRecv == WAIT_OBJECT_0)
	{
		cout << "Recv ������ ����\n";
	}
	else if (retvalRecv == WAIT_TIMEOUT)	//�������̶� �Ȱɸ�
	{
		cout << "Ÿ�Ӿƿ�\n";
	}
	else
	{
		cout << "���� �߻�\n";
	}

	cout << "MainThread ����\n";

	

	return 0;
}