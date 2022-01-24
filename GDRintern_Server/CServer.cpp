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

	m_fX = 0;
	m_fY = 0;
	m_fZ = 0;

	m_bState = false;

	m_iPhase = 0;
	m_fBallSpeed = 1.f;
	m_fLaunchAngle = 2.f;
	m_fLaunchDirection = 3.f;
	m_fHeadSpeed = 4.f;
	m_iBackSpin = 5;
	m_iSideSpin = 6;
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

void CServer::ReadData(const unsigned int type)
{
	CServer& server = CServer::Instance();

	if (type == PT_Connect)		//대기 시 통신
	{
		std::cout << "PT_Connect recv\n";
	}
	else if (type == PT_Setting)	//Tee, Club설정 변경
	{
		std::cout << "PT_Setting recv\n";
		
		TCSetting tcs;
		ServerRecv(&tcs, sizeof(TCSetting));

		server.m_hMutex.lock();
		server.SetTCSetting(tcs);
		server.m_hMutex.unlock();

		std::cout << server.m_eTee << "   " << server.m_eClub << "\n";
		
		Packet pt(PT_None, sizeof(Packet));
		ServerSend(&pt, NULL, 0);
		return;
	}
	else if (type == PT_Active)		//Active 상태 변경
	{
		std::cout << "PT_Active recv\n";

		ACTIVESTATE activestate;		//가변데이터 state 상태 수신
		ServerRecv(&activestate, sizeof(ACTIVESTATE));

		server.m_hMutex.lock();
		server.m_bState = activestate.state;
		server.m_hMutex.unlock();
		
		Packet pt(PT_Pos, sizeof(Packet));		//공 위치 정보 send
		POS pos{0,0,0};
		ServerSend(&pt, &pos, sizeof(pos));
		return;
	}
	else if (type == PT_Shot)		//샷을 진행했다는 알람(pc -> pc에만 적용사항)
	{
		std::cout << "PT_Shot recv\n";

		Packet pt(PT_ShotData, sizeof(Packet));
		ShotData shotdata{ server.GetShotData() };		//샷 데이터 send
		ServerSend(&pt, &shotdata, sizeof(ShotData));

		server.m_bState = false;		//샷 후 inactive 상태 변경
		return;							
	}
	else
	{
		std::cout << "recv ok\n";
	}

	return;
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	CServer& server = CServer::Instance();

	std::cout << "ConnectInit\n" << std::endl;

	Packet pt{ PT_Connect, sizeof(Packet) };
	while (true)
	{
		//if (SOCKET_ERROR == server.ServerSend(&pt, NULL, 0))
		//{
		//	std::cout << "SendThread ServerSend error\n";
		//	//err_quit("send()");
		//	break;
		//}//Set_Packet->Server_Send

		//std::cout << "Send OK\n";

	}
	return NULL;
}

DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	CServer& server = CServer::Instance();
	Packet pt;
	while (true)
	{

		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == server.ServerRecv(&pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		server.ReadData(pt.type);


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

		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept 성공 시

		DWORD dwSendThreadID, dwRecvThreadID;
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(server.m_hClient);
	}
	return;
}

int CServer::ServerSend(const void* fixbuf, const void* varbuf, const int varlen)
{
	CServer& server = CServer::Instance();
	int retval{ 0 };

	retval = send(server.m_hClient, (const char*)fixbuf, sizeof(Packet), 0);	//고정데이터 전송
	if (SOCKET_ERROR == retval)
	{
		std::cout << "ServerSend error fixbuf send\n";
	}
	else
	{
		if (0 != varlen)		//가변 데이터에 무언가 있어 추가 전송
		{
			retval = send(server.m_hClient, (const char*)varbuf, varlen, 0);
			if (SOCKET_ERROR == retval)
			{
				std::cout << "ServerSend error varbuf send\n";
			}
		}
	}
	return retval;
}
int CServer::ServerRecv(void* buf, const int len)
{
	CServer& server = CServer::Instance();
	return recv(server.m_hClient, (char*)buf, len, 0);
}


