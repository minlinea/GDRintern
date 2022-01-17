#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include "stdio.h"
#include <WinSock2.h>
#include "windows.h"
using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 4578
#define PACKET_SIZE 1024
#define SERVER_IP "127.0.0.1"

HANDLE hSend;
HANDLE hRecv;
HANDLE hMutex;

DWORD WINAPI SendThread(LPVOID socket)//积己等 家南 逞败林扁
{
	int retval = 0;
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);
		int type;
		string sinput;
		cout << "input : ";
		getline(cin, sinput);
		retval = send((SOCKET)socket, sinput.c_str(), PACKET_SIZE, 0);
		cout << "retval size = " << retval << "\n";
		if (retval == SOCKET_ERROR)
		{
			cout << "send error\n";
			break;
		}
		cout << sinput << " send\n";
		ReleaseMutex(hMutex);
	}
	
	return NULL;
}

DWORD WINAPI RecvThread(LPVOID socket)
{
	int retval = 0;
	while (true)
	{
		WaitForSingleObject(hMutex, INFINITE);

		char cBuffer[PACKET_SIZE] = {};
		retval = recv((SOCKET)socket, cBuffer, PACKET_SIZE, 0);

		cout << "Recv Msg : " << cBuffer << "\n";

		if (retval == SOCKET_ERROR)
		{
			cout << "recv error\n";
			break;
		}
		ReleaseMutex(hMutex);
	}
	return NULL;
}



int main()
{
	hMutex = (HANDLE)CreateMutex(NULL, FALSE, NULL);

	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET hSocket;
	hSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN tAddr = {};
	tAddr.sin_family = AF_INET;
	tAddr.sin_port = htons(PORT);
	tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);


	int retval = connect(hSocket, (SOCKADDR*)&tAddr, sizeof(tAddr));
	if (retval == SOCKET_ERROR)
	{
		cout << "send error" << endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		cout << "MainThread 矫累\n";

		hSend = CreateThread(0, 0, SendThread, (LPVOID)hSocket, 0, &dwSendThreadID);
		hRecv = CreateThread(0, 0, RecvThread, (LPVOID)hSocket, 0, &dwRecvThreadID);


		WaitForSingleObject(hRecv, INFINITE);
	}
	
	closesocket(hSocket);

	WSACleanup();
	return 0;
}