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

void CServer::ReadData(unsigned int type)
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
		ServerRecv(server.m_hClient, &tcs, sizeof(TCSetting));
		server.SetTCSetting(tcs);

		std::cout << server.m_eTee << "   " << server.m_eClub << "\n";
		
		Packet pt(sizeof(Packet), PT_None);
		ServerSend(server.m_hClient, &pt, NULL, 0);
		return;
	}
	else if (type == PT_Active)		//Active 상태 변경
	{
		std::cout << "PT_Active recv\n";

		ACTIVESTATE activestate;		//가변데이터 state 상태 수신
		ServerRecv(server.m_hClient, &activestate, sizeof(ACTIVESTATE));
		server.m_bState = activestate.state;

		Packet pt(PT_Pos, sizeof(POS) + sizeof(Packet));		//공 위치 정보 send
		POS pos = {0,0,0};
		ServerSend(server.m_hClient, &pt, &pos, sizeof(pos));
		return;
	}
	else if (type == PT_Shot)		//샷을 진행했다는 알람(pc -> pc에만 적용사항)
	{
		std::cout << "PT_Shot recv\n";

		Packet pt(PT_ShotData, sizeof(ShotData));
		ShotData shotdata(server.GetShotData());		//샷 데이터 send
		ServerSend(server.m_hClient, &pt, &shotdata, sizeof(ShotData));

		server.m_bState = false;		//샷 후 inactive 상태 변경
		return;							//유일하게 다른 동작이므로 아래 상태를 진행하지 않음
	}
	else
	{
		std::cout << "recv ok\n";
	}
}

void CServer::ConnectInit()	
{
	CServer& server = CServer::Instance();
	
	Packet pt(PT_Pos, sizeof(Packet));
	POS pos = { -1,-2,-3 };
	ServerSend(server.m_hClient, &pt, &pos, sizeof(POS));
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	CServer& server = CServer::Instance();

	server.ConnectInit();
	std::cout << "ConnectInit\n" << std::endl;

	Packet pt{ PT_Connect, sizeof(Packet) };
	while (true)
	{
		std::lock_guard<std::mutex> lock(server.m_hMutex);

		if (SOCKET_ERROR == server.ServerSend((SOCKET)socket, &pt, NULL, 0))
		{
			std::cout << "SendThread ServerSend error\n";
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

		DWORD retvalSend = WaitForSingleObject(server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(server.m_hClient);
	}
	return;
}

int CServer::ServerSend(const SOCKET& sock, const void* fixbuf, const void* varbuf, int varlen)
{
	int retval = 0;

	retval = send(sock, (const char*)fixbuf, sizeof(Packet), 0);	//고정데이터 전송
	if (SOCKET_ERROR == retval)
	{
		std::cout << "ServerSend error fixbuf send\n";
	}
	else
	{
		if (0 != varlen)		//가변 데이터에 무언가 있어 추가 전송
		{
			retval = send(sock, (const char*)varbuf, varlen, 0);
			if (SOCKET_ERROR == retval)
			{
				std::cout << "ServerSend error varbuf send\n";
			}
		}
	}
	return retval;
}
int CServer::ServerRecv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}


