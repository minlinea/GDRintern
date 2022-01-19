#include "CServer.h"
#include <iostream>

CServer::CServer()
{

}

CServer::~CServer()
{
	closesocket(this->m_hListen);
	CloseHandle(this->m_hSend);
	CloseHandle(this->m_hRecv);
	//closehandle(this->m_hMutex);
	WSACleanup();
}

void CServer::ServerInit()
{
	WSAStartup(MAKEWORD(2, 2), &this->m_wsaData);

	this->m_hListen = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	this->m_tListenAddr.sin_family = AF_INET;
	this->m_tListenAddr.sin_port = htons(PORT);
	this->m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(this->m_hListen, (SOCKADDR*)&this->m_tListenAddr, sizeof(this->m_tListenAddr));

	listen(this->m_hListen, SOMAXCONN);
	std::cout << "serverInit\n";
}

DWORD WINAPI CServer::ListenThread(LPVOID socket)
{
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
	while (true)
	{
		std::cout << "ServerAccept\n";
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		this->m_hClient = accept(this->m_hListen, (SOCKADDR*)&tCIntAddr, &iCIntSize);		//클라이언트가 죽어도 얘는 통신 대기.
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;

		DWORD dwSendThreadID, dwRecvThreadID;

	

		this->m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CServer::SendThread, (LPVOID)this->m_hClient, 0, &dwSendThreadID);

		this->m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CServer::RecvThread, (LPVOID)this->m_hClient, 0, &dwRecvThreadID);

		std::thread t(&CServer::SendThread, (LPVOID)this->m_hClient);

		DWORD retvalSend = WaitForSingleObject(this->m_hSend, INFINITE);
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(this->m_hClient);
	}

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

