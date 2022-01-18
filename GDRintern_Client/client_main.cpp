#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include "stdio.h"
#include <WinSock2.h>
#include "windows.h"
#include "../packet.h"

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
void Client_Send(const SOCKET& sock, const void* buf, int len);
void Client_Recv(const SOCKET& sock, void* buf, int len);
void Set_Packet(const SOCKET& sock, unsigned int type);

DWORD WINAPI SendThread(LPVOID socket)
{
	string sinput;
	int retval = 0;
	while (true)
	{
		lock_guard<mutex> lock(m);
		int type;
		ZeroMemory(&sinput, sinput.size());
		ZeroMemory(&cBuffer, PACKET_SIZE);

		cout << "input : ";
		getline(cin, sinput);
		strcpy(cBuffer, sinput.c_str());

		Set_Packet((SOCKET)socket, PT_Connect);

		//ReleaseMutex(hMutex);
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

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		Client_Recv((SOCKET)socket, &pt, sizeof(pt));

		cout << "Recv check\n";
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

void Client_Send(const SOCKET& sock, const void* buf, int len)
{
	if (SOCKET_ERROR == send(sock, (const char*)buf, len, 0))
	{
		cout << "Client_Send error\n";
		err_quit("Client_Send()");
		return;
	}
	else
	{

	}
	return;
}

void Client_Recv(const SOCKET& sock, void* buf, int len)
{
	if (SOCKET_ERROR == recv(sock, (char*)buf, len, 0))
	{
		cout << "Client_Recv error\n";
		err_quit("Client_Recv()");
		return;
	}
	else
	{

	}
	return;
}

void Set_Packet(const SOCKET& sock, unsigned int type)
{
	Packet pt;

	if (type == PT_Connect)
	{
		pt.type = PT_Connect;
	}
	else if (type == PT_Active)
	{
		pt.type = PT_Active;
	}
	else if (type == PT_Setting)
	{
		pt.type = PT_Setting;
	}
	else if (type == PT_ConnectCheck)
	{
		pt.type = PT_ConnectCheck;
	}
	else if (type == PT_Disconnect)
	{
		pt.type = PT_Disconnect;
	}
	else if (type == PT_None)
	{
		pt.type = PT_Disconnect;
	}
	else
	{
		cout << "Set_packet error : error type\n";
		return;
	}

	Client_Send(sock, &pt, sizeof pt);

	return;
}