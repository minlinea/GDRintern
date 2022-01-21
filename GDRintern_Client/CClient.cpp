#include "CClient.h"
#include <string>
#include <iostream>

CClient::CClient()
{
	DataInit();
}

CClient::~CClient()
{
	closesocket(Instance().m_hSock);
	CloseHandle(Instance().m_hSend);
	CloseHandle(Instance().m_hRecv);
	//CloseHandle(Instance().m_hMutex);
	WSACleanup();
}

void CClient::DataInit()
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

bool CClient::ClientInit()
{
	CClient& client = CClient::Instance();
	if (0 != WSAStartup(MAKEWORD(2, 2), &client.m_wsaData))
	{
		std::cout << "ClientInit WSAStartup fail\n";
		return false;
	}

	client.m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == client.m_hSock)
	{
		std::cout << "ClientInit ListenSocket fail\n";
		return false;
	}

	client.m_tAddr.sin_family = AF_INET;
	client.m_tAddr.sin_port = htons(PORT);
	client.m_tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
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
	CClient& client = CClient::Instance();
	std::string sinput;
	int retval = 0;
	while (true)
	{
		//std::lock_guard<std::mutex> lock(client.m_hMutex);
		ZeroMemory(&sinput, sinput.size());

		std::cout << "input : ";
		std::getline(std::cin, sinput);

		if (SOCKET_ERROR == client.SetPacket((SOCKET)socket, PT_Connect))
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
	CClient& client = CClient::Instance();
	int retval = 0;
	while (true)
	{
		//std::cout << "RecvThread Client\n";
		//std::lock_guard<std::mutex> lock(client.m_hMutex);
		//std::cout << "RecvThread\n";
		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == client.ClientRecv((SOCKET)client.m_hSock, &pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		std::cout << "size : " << pt.size << "\n";
		if (pt.size != sizeof(Packet))
		{
			POS pos;
			ZeroMemory(&pos, sizeof(pos));
			client.ClientRecv((SOCKET)client.m_hSock, &pos, sizeof(pos));
			
			std::cout << pos.x << std::endl;
			std::cout << pos.y << std::endl;
			std::cout << pos.z << std::endl;
		}
		std::cout << "Recv Ok\n";
	}
	return NULL;
}

void CClient::ClientConnect()
{
	CClient& client = CClient::Instance();
	int retval = connect(client.m_hSock, (SOCKADDR*)&client.m_tAddr, sizeof(client.m_tAddr));
	if (retval == SOCKET_ERROR)
	{
		std::cout << "connect error" << std::endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		std::cout << "MainThread ½ÃÀÛ\n";

		m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client.SendThread, (LPVOID)client.m_hSock, 0, &dwSendThreadID);
		m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client.RecvThread, (LPVOID)client.m_hSock, 0, &dwRecvThreadID);


		WaitForSingleObject(m_hRecv, INFINITE);
	}
	closesocket(m_hSock);

	return;
}

int CClient::ClientSend(const SOCKET& sock, const void* buf, int len)
{
	return send(sock, (const char*)buf, len, 0);
}

int CClient::ClientRecv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int CClient::SetPacket(const SOCKET& sock, unsigned int type)
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

	return ClientSend(sock, &pt, sizeof pt);
}

int CClient::recvn(SOCKET s, char* buf, int len, int flags)

{
	CClient& client = CClient::Instance();
	int received;
	char* ptr = buf;
	int left = len;
	while (left > 0)
	{
		received = recv(client.m_hSock, ptr, left, flags);
		if (received == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else if (received == 0)
		{
			break;
		}
		left -= received;
		ptr += received;
	}
	return (len - left);
}
