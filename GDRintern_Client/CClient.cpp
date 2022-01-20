#include "CClient.h"
#include <string>
#include <iostream>

CClient::CClient()
{

}

CClient::~CClient()
{
	closesocket(Instance().m_hSock);
	CloseHandle(Instance().m_hSend);
	CloseHandle(Instance().m_hRecv);
	//CloseHandle(Instance().m_hMutex);
	WSACleanup();
}

bool CClient::ClientInit()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &Instance().m_wsaData))
	{
		std::cout << "ClientInit WSAStartup fail\n";
		return false;
	}

	Instance().m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Instance().m_hSock)
	{
		std::cout << "ClientInit ListenSocket fail\n";
		return false;
	}

	Instance().m_tAddr.sin_family = AF_INET;
	Instance().m_tAddr.sin_port = htons(PORT);
	Instance().m_tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	return true;
}

void CClient::err_quit(const char* msg)
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

DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	std::string sinput;
	int retval = 0;
	while (true)
	{
		std::lock_guard<std::mutex> lock(Instance().m_hMutex);
		ZeroMemory(&sinput, sinput.size());

		std::cout << "input : ";
		std::getline(std::cin, sinput);

		if (SOCKET_ERROR == Instance().Set_Packet((SOCKET)socket, PT_Connect))
		{
			std::cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}//Set_Packet->Client_Send
	}

	return NULL;
}

DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	int retval = 0;
	while (true)
	{
		std::lock_guard<std::mutex> lock(Instance().m_hMutex);

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == Instance().Client_Recv((SOCKET)socket, &pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		std::cout << "Recv Ok\n";
	}
	return NULL;
}

void CClient::ClientConnect()
{
	int retval = connect(Instance().m_hSock, (SOCKADDR*)&Instance().m_tAddr, sizeof(Instance().m_tAddr));
	if (retval == SOCKET_ERROR)
	{
		std::cout << "connect error" << std::endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		std::cout << "MainThread ½ÃÀÛ\n";

		m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CClient::Instance().SendThread, (LPVOID)Instance().m_hSock, 0, &dwSendThreadID);
		m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CClient::Instance().RecvThread, (LPVOID)Instance().m_hSock, 0, &dwRecvThreadID);


		WaitForSingleObject(m_hRecv, INFINITE);
	}
	closesocket(m_hSock);

	return;
}

int CClient::Client_Send(const SOCKET& sock, const void* buf, int len)
{
	return send(sock, (const char*)buf, len, 0);
}

int CClient::Client_Recv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int CClient::Set_Packet(const SOCKET& sock, unsigned int type)
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
		std::cout << "Set_packet error : error type\n";
		return -1;
	}

	return Client_Send(sock, &pt, sizeof pt);
}

