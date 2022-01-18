#include <iostream>
#include <thread>
#include "windows.h"
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 4578
#define PACKET_SIZE 1024

char cBuffer[PACKET_SIZE] = {};
HANDLE hSend;
HANDLE hRecv;
HANDLE hMutex;
DWORD WINAPI SendThread(LPVOID socket)//������ ���� �Ѱ��ֱ�
{
	cout << "SendThread\n";
	int retval = 0;
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);

		retval = send((SOCKET)socket, cBuffer, PACKET_SIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			cout << "send error\n";
			break;
		}
		cout << "Send check\n";

		ReleaseMutex(hMutex);
	}
	cout << "SendThread End\n";
	return NULL;
}

DWORD WINAPI RecvThread(LPVOID socket)
{
	cout << "RecvThread\n";
	int retval = 0;
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);

		retval = recv((SOCKET)socket, cBuffer, PACKET_SIZE, 0);

		string sBuffer;
		sBuffer = cBuffer;
		cout << "Recv Msg : " << sBuffer << endl;

		if (retval == SOCKET_ERROR)
		{
			cout << "recv error\n";
			break;
		}

		cout << "Recv check\n";

		ReleaseMutex(hMutex);
	}
	cout << "RecvThread End\n";
	return NULL;
}



int main()
{
	hMutex = (HANDLE)CreateMutex(NULL, FALSE, NULL);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET hListen;
	hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN tListenAddr = {};
	tListenAddr.sin_family = AF_INET;
	tListenAddr.sin_port = htons(PORT);
	tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(hListen, (SOCKADDR*)&tListenAddr, sizeof(tListenAddr));
	listen(hListen, SOMAXCONN);

	SOCKET hClient;
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		hClient = accept(hListen, (SOCKADDR*)&tCIntAddr, &iCIntSize);		//Ŭ���̾�Ʈ�� �׾ ��� ��� ���.
		cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << endl;

		DWORD dwSendThreadID, dwRecvThreadID;

		hSend = CreateThread(0, 0, RecvThread, (LPVOID)hClient, 0, &dwSendThreadID);

		hRecv = CreateThread(0, 0, SendThread, (LPVOID)hClient, 0, &dwRecvThreadID);






		DWORD retvalSend = WaitForSingleObject(hSend, INFINITE);
		cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << endl;
		closesocket(hClient);
	}

	closesocket(hListen);
	CloseHandle(hMutex);
	WSACleanup();

	return 0;
}