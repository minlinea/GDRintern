#include "CServer.h"
#include <iostream>

CServer::CServer()
{

}

CServer::~CServer()
{
	closesocket(Instance().m_hListenSock);
	CloseHandle(Instance().m_hSend);
	CloseHandle(Instance().m_hRecv);
	CloseHandle(Instance().m_hListen);
	//CloseHandle(Instance().m_hMutex);
	WSACleanup();
}

void CServer::ServerInit()
{
	WSAStartup(MAKEWORD(2, 2), &this->m_wsaData);

	Instance().m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	Instance().m_tListenAddr.sin_family = AF_INET;
	Instance().m_tListenAddr.sin_port = htons(PORT);
	Instance().m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(Instance().m_hListenSock, (SOCKADDR*)&Instance().m_tListenAddr, sizeof(Instance().m_tListenAddr));

	listen(Instance().m_hListenSock, SOMAXCONN);
}

void CServer::err_quit(const char* msg)
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

DWORD WINAPI CServer::ListenThread(LPVOID socket)
{
	SOCKET t = Instance().m_hListenSock;
	std::cout << "Server Listen\n";
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		Instance().m_hClient = accept(Instance().m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;

		DWORD dwSendThreadID, dwRecvThreadID;
		Instance().m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CServer::Instance().RecvThread, (LPVOID)Instance().m_hClient, 0, &dwRecvThreadID);
		Instance().m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CServer::Instance().SendThread, (LPVOID)Instance().m_hClient, 0, &dwSendThreadID);

		DWORD retvalSend = WaitForSingleObject(Instance().m_hSend, INFINITE);
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(Instance().m_hClient);
	}
	return NULL;
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	while (true)
	{
		std::lock_guard<std::mutex> lock(Instance().m_hMutex);

		if (SOCKET_ERROR == Instance().Set_Packet((SOCKET)socket, PT_Connect))
		{
			std::cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}//Set_Packet->Server_Send

		std::cout << "Send OK\n";

	}
	return NULL;
}

DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	while (true)
	{
		std::lock_guard<std::mutex> lock(Instance().m_hMutex);

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == Instance().Server_Recv((SOCKET)socket, &pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		std::cout << "Recv Ok\n";

		
		//에코용
		if (SOCKET_ERROR == Instance().Set_Packet((SOCKET)socket, PT_Connect))
		{
			std::cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}
		std::cout << "Send OK\n";
		//에코용 Set_Packet->Server_Send

	}
	return NULL;
}

void CServer::ServerAccept()
{
	DWORD dwListenThreadID;
	this->m_hListen = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CServer::ListenThread,
										(LPVOID)Instance().m_hClient, 0, &dwListenThreadID);
}

int CServer::Server_Send(const SOCKET& sock, const void* buf, int len)
{
	return send(sock, (const char*)buf, len, 0);
}

int CServer::Server_Recv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int CServer::Set_Packet(const SOCKET& sock, unsigned int type)
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
		std::cout << "Set_packet error : error type\n";
		return -1;
	}

	std::cout << "Server Send\n";

	return Server_Send(sock, &pt, sizeof pt);
}

