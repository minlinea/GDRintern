#include <iostream>
#include <thread>
#include "windows.h"
using namespace std;

DWORD WINAPI SendThread(LPVOID socket)//생성된 소켓 넘겨주기
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

	cout << "MainThread 시작\n";

	HANDLE hSend = CreateThread(0, 0, SendThread, NULL, 0, &dwSendThreadID);

	DWORD retvalSend = WaitForSingleObject(hSend, INFINITE);
	if (retvalSend == WAIT_OBJECT_0)
	{
		cout << "Send 스레드 종료\n";
	}
	else if (retvalSend == WAIT_TIMEOUT)	//무제한이라 안걸림
	{
		cout << "타임아웃\n";
	}
	else
	{
		cout << "에러 발생\n";
	}
	HANDLE hRecv = CreateThread(0, 0, RecvThread, NULL, 0, &dwRecvThreadID);

	DWORD retvalRecv = WaitForSingleObject(hRecv, INFINITE);
	if (retvalRecv == WAIT_OBJECT_0)
	{
		cout << "Recv 스레드 종료\n";
	}
	else if (retvalRecv == WAIT_TIMEOUT)	//무제한이라 안걸림
	{
		cout << "타임아웃\n";
	}
	else
	{
		cout << "에러 발생\n";
	}

	cout << "MainThread 종료\n";

	

	return 0;
}