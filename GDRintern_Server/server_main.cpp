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
int Server_Send(const SOCKET& sock, const void* buf, int len);
int Server_Recv(const SOCKET& sock, void* buf, int len);
int Set_Packet(const SOCKET& sock, unsigned int type);

DWORD WINAPI SendThread(LPVOID socket)//생성된 소켓 넘겨주기
{
	while (true)
	{
		lock_guard<mutex> lock(m);

		if (SOCKET_ERROR == Set_Packet((SOCKET)socket, PT_Connect))
		{
			cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}//Set_Packet->Server_Send

		cout << "Send OK\n";

	}
	return NULL;
}

DWORD WINAPI RecvThread(LPVOID socket)
{
	while (true)
	{
		lock_guard<mutex> lock(m);

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == Server_Recv((SOCKET)socket, &pt, sizeof(Packet)))
		{
			cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		cout << "Recv Ok\n";


		//에코용
		if (SOCKET_ERROR == Set_Packet((SOCKET)socket, PT_Connect))
		{
			cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}
		cout << "Send OK\n";
		//에코용 Set_Packet->Server_Send
		 
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




int Server_Send(const SOCKET& sock, const void* buf, int len)
{
	return send(sock, (const char*)buf, len, 0);
}

int Server_Recv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int Set_Packet(const SOCKET& sock, unsigned int type)
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
		return -1;
	}

	cout << "Server Send\n";

	return Server_Send(sock, &pt, sizeof pt);
}