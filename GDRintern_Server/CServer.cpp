#include "CServer.h"
#include <iostream>

CServer::CServer()
{
	DataInit();
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

void CServer::DataInit()
{
	m_eTee = T40;
	m_eClub = DRIVER;

	m_fX = -1;
	m_fY = -1;
	m_fZ = -1;

	m_bState = false;

	m_iPhase = 0;
	m_fBallSpeed = 0.f;
	m_fLaunchAngle = 0.f;
	m_fLaunchDirection = 0.f;
	m_fHeadSpeed = 0.f;
	m_iBackSpin = 0;
	m_iSideSpin = 0;
}

bool CServer::ServerInit()
{
	CServer& server = CServer::Instance();
	if (0 != WSAStartup(MAKEWORD(2, 2), &server.m_wsaData))
	{
		std::cout << "ServerInit WSAStartup fail\n";
		return false;
	}

	server.m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == server.m_hListenSock)
	{
		std::cout << "ServerInit ListenSocket fail\n";
		return false;
	}

	server.m_tListenAddr.sin_family = AF_INET;
	server.m_tListenAddr.sin_port = htons(PORT);
	server.m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(server.m_hListenSock, (SOCKADDR*)&server.m_tListenAddr, sizeof(server.m_tListenAddr)))
	{
		std::cout << "ServerInit bind fail\n";
		return false;
	}

	if (SOCKET_ERROR == listen(server.m_hListenSock, SOMAXCONN))
	{
		std::cout << "ServerInit listen fail\n";
		return false;
	}

	return true;
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

void CServer::ConnectInit()	
{
	CServer& server = CServer::Instance();
	
	Packet pt(PT_Pos, sizeof(POS) + sizeof(Packet));
	POS pos = { -1,-2,-3 };
	ServerSend(server.m_hClient, &pt, sizeof(Packet));
	ServerSend(server.m_hClient, &pos, sizeof(pos));
	



	//Packet* pt = (Packet*)malloc(sizeof(Packet) + sizeof(POS));
	//pt->SetSize(sizeof(Packet) + sizeof(POS));
	//pt->SetType(PT_Pos);
	//pt->SetVariableData(sizeof(POS), &pos);
	//ServerSend(server.m_hClient, pt, pt->size);
	//std::cout << " ServerSend\n" << std::endl;
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	CServer& server = CServer::Instance();


	std::cout << "ConnectInit\n" << std::endl;
	while (true)
	{
		std::lock_guard<std::mutex> lock(server.m_hMutex);

		if (SOCKET_ERROR == server.SetPacket((SOCKET)socket, PT_Connect))
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
	CServer& server = CServer::Instance();
	while (true)
	{
		std::lock_guard<std::mutex> lock(server.m_hMutex);

		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == server.ServerRecv((SOCKET)socket, &pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		std::cout << "Recv Ok\n";

		
		if(SOCKET_ERROR == send((SOCKET)socket, (const char*)&pt, sizeof(Packet), 0))
		{
			std::cout << "Set_Packet error\n";
			//err_quit("send()");
			break;
		}
		//���ڿ�
		//if (SOCKET_ERROR == server.SetPacket((SOCKET)socket, PT_Connect))
		//{
		//	std::cout << "Set_Packet error\n";
		//	//err_quit("send()");
		//	break;
		//}
		std::cout << "Send OK\n";
		//���ڿ� Set_Packet->Server_Send

	}
	return NULL;
}

void CServer::ServerAccept()
{
	CServer& server = CServer::Instance();
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		server.m_hClient = accept(server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);



		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept ���� ��

		DWORD dwSendThreadID, dwRecvThreadID;
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		server.ConnectInit();

		DWORD retvalSend = WaitForSingleObject(server.m_hSend, INFINITE);//Send������ ���� ���(Ŭ���̾�Ʈ���� ���� ���� ���� Ȯ��)
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(server.m_hClient);
	}
	return;
}

int CServer::ServerSend(const SOCKET& sock, const void* buf, int len)
{
	return send(sock, (const char*)buf, len, 0);
}

int CServer::ServerRecv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int CServer::SetPacket(const SOCKET& sock, unsigned int type)
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

	return ServerSend(sock, &pt, sizeof(pt));
}
