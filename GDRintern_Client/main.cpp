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
mutex m;
mutex m2;
char cBuffer[PACKET_SIZE] = {};

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER
		| FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);

}

DWORD WINAPI SendThread(LPVOID socket)
{
	string sinput;
	int retval = 0;
	while (true)
	{
		//WaitForSingleObject(hMutex, INFINITE);
		lock_guard<mutex> lock(m);
		int type;
		ZeroMemory(&sinput, sinput.size());
		ZeroMemory(&cBuffer, PACKET_SIZE);

		cout << "input : ";
		getline(cin, sinput);
		strcpy(cBuffer, sinput.c_str());
		retval = send((SOCKET)socket, cBuffer, PACKET_SIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			cout << "send error\n";
			err_quit("send()");
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
		//WaitForSingleObject(hMutex, INFINITE);
		lock_guard<mutex> lock(m);
		retval = recv((SOCKET)socket, cBuffer, PACKET_SIZE, 0);
		if (retval == SOCKET_ERROR)
		{
			cout << "recv error\n";
			err_quit("recv()");
			break;
		}
		cout << "Recv Msg : " << cBuffer << "\n";

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

		cout << "MainThread ½ÃÀÛ\n";
		
		hSend = CreateThread(0, 0, SendThread, (LPVOID)hSocket, 0, &dwSendThreadID);
		hRecv = CreateThread(0, 0, RecvThread, (LPVOID)hSocket, 0, &dwRecvThreadID);


		WaitForSingleObject(hRecv, INFINITE);
	}
	
	closesocket(hSocket);

	CloseHandle(hMutex);
	WSACleanup();
	return 0;
}