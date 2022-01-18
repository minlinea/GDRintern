#include <iostream>
#include <thread>
#include <mutex>
#include "windows.h"
#include "../packet.h"

using namespace std;

#pragma comment(lib, "ws2_32")

#define PORT 4578
#define PACKET_SIZE 1024

char cBuffer[PACKET_SIZE] = {};
HANDLE hSend;
HANDLE hRecv;
HANDLE hMutex;
mutex m;

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
void Server_Send(const SOCKET& sock, const void* buf, int len);
void Server_Recv(const SOCKET& sock, void* buf, int len);
void Set_Packet(const SOCKET& sock, unsigned int type);

DWORD WINAPI SendThread(LPVOID socket)//생성된 소켓 넘겨주기
{
	while (true)
	{
		//WaitForSingleObject(hMutex, INFINITE);
		lock_guard<mutex> lock(m);

		Set_Packet((SOCKET)socket, PT_Connect);	//Set_Packet->Server_Send

		cout << "Send check\n";

		//ReleaseMutex(hMutex);
	}
	return NULL;
}

DWORD WINAPI RecvThread(LPVOID socket)
{
	while (true)
	{
		//WaitForSingleObject(hMutex, INFINITE);
		lock_guard<mutex> lock(m);

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		Server_Recv((SOCKET)socket, &pt, sizeof(Packet));

		Set_Packet((SOCKET)socket, PT_Connect);	//Set_Packet->Server_Send
																//에코용

		cout << "Recv check\n";

		//ReleaseMutex(hMutex);
	}
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
		hClient = accept(hListen, (SOCKADDR*)&tCIntAddr, &iCIntSize);		//클라이언트가 죽어도 얘는 통신 대기.
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




void Server_Send(const SOCKET& sock, const void* buf, int len)
{
	if (SOCKET_ERROR == send(sock, (const char*)buf, len, 0))
	{
		cout << "Server_Send error\n";
		err_quit("Server_Send()");
		return;
	}
	else
	{

	}

	return;
}

void Server_Recv(const SOCKET& sock, void* buf, int len)
{
	if (SOCKET_ERROR == recv(sock, (char*)buf, len, 0))
	{
		cout << "Server_Recv error\n";
		err_quit("Server_Recv()");
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
	else if (type == PT_Pos)
	{
		pt.type = PT_Pos;
	}
	else if (type == PT_ShotData)
	{
		pt.type = PT_ShotData;
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
		pt.type = PT_None;
	}
	else
	{
		cout << "Set_packet error : error type\n";
		return;
	}

	Server_Send(sock, &pt, sizeof pt);
	cout << "Server Send\n";
	return;
}