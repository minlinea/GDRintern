#include "CServer.h"
#include "conio.h"

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
	m_eTee = TEESETTING::T40;

	m_eClub = CLUBSETTING::DRIVER;

	m_eBallPlace = BALLPLACE::OB;

	m_bActiveState = false;

	m_sdShotData = ShotData{ 0,1,2,3,4,5,6 };
}

bool CServer::ServerInit()
{
	auto& server = CServer::Instance();
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

void CServer::ReadData(Packet packet)
{
	auto& server = CServer::Instance();
	if (sizeof(Packet) == packet.GetSize())
	{
		if (PACKETTYPE::PT_ShotDataRecv == packet.GetType())
		{
			std::cout << "PT_ShotDataRecv recv\n";
		}
		else if (PACKETTYPE::PT_ConnectRecv == packet.GetType())
		{
			std::cout << "PT_ConnectRecv recv\n";
		}
		else
		{
			std::cout << "ReadData unknown type\n";
		}
	}
	else
	{
		packet.SetRecvData();
		if (SOCKET_ERROR == server.ServerRecv(packet.GetData(), packet.GetSize()))
		{
			std::cout << "ReadData ClientRecv\n";
		}
		else
		{
			if (PACKETTYPE::PT_ClubSetting == packet.GetType())
			{
				std::cout << "PT_ClubSetting recv\n";
				server.SetClubSetting(packet.GetData());
			}
			else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
			{
				std::cout << "PT_TeeSetting recv\n";
				server.SetTeeSetting(packet.GetData());
			}
			else if (PACKETTYPE::PT_ActiveState == packet.GetType())
			{
				std::cout << "PT_BallPlace Recv\n";
				server.SetActiveState(packet.GetData());
			}
			else
			{
				std::cout << "ReadData unknown type\n";
			}
		}
		packet.DeleteData();
	}
}

int CServer::InputKey(const char input)
{
	auto& server = CServer::Instance();
	int retval{ 1 };
	//Packet<nullptr_t> pt;
	if ('w' == input)		//ShotData 전달
	{		
		Packet pt(PACKETTYPE::PT_BallPlace, server.GetBallPlace());
		server.ServerSend(pt);
		std::cout << "PT_BallPlace send\n";
	}
	else if ('e' == input)		
	{
		Packet pt(PACKETTYPE::PT_ShotData, server.GetShotData());
		server.ServerSend(pt);
		std::cout << "PT_ShotData send\n";
	}
	else if ('r' == input)
	{
		server.SetActiveState(false);

		Packet pt(PACKETTYPE::PT_ActiveState, server.GetActiveState());
		server.ServerSend(pt);
		std::cout << "PT_ActiveState(false) send\n";
	}
	else
	{
	}
	//server.ServerSend(&pt, sizeof(pt));
	return retval;
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	std::cout << "ConnectInit\n" << std::endl;

	//Packet<void> pt{ PACKETTYPE::PT_Connect};
	while (true)
	{
		if (true == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		{
			if (SOCKET_ERROR == server.InputKey(_getch()))
			{
				std::cout << "SendThread InputKey error\n";
				//err_quit("send()");
				break;
			}//InputKey->ServerSend
		}
		else	//유휴상태 추가
		{

		}

	}
	return NULL;
}

DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	auto& server = CServer::Instance();
	
	while (true)
	{
		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == server.ServerRecv(&pt, sizeof(pt)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		else    //에러가 아니라면 데이터 읽기
		{
			server.ReadData(pt);
		}
	}
	return NULL;
}

void CServer::ServerAccept()
{
	auto& server = CServer::Instance();
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		server.m_hClient = accept(server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);

		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept 성공 시

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv 스레드 생성
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
		closesocket(server.m_hClient);
	}
	return;
}

int CServer::ServerSend(Packet& packet)
{
	auto& server = CServer::Instance();
	return send(server.m_hClient, (const char*)packet.GetData(), packet.GetSize(), 0);
}
int CServer::ServerRecv(void* buf, const int len)
{
	auto& server = CServer::Instance();
	return recv(server.m_hClient, (char*)buf, len, 0);
}


