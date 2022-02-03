#include "CServer.h"
#include "conio.h"

CLog clog;

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
		clog.Log("ERROR", "ServerInit WSAStartup fail");
		return false;
	}

	server.m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == server.m_hListenSock)
	{
		clog.Log("ERROR", "ServerInit ListenSocket fail");
		return false;
	}

	server.m_tListenAddr.sin_family = AF_INET;
	server.m_tListenAddr.sin_port = htons(PORT);
	server.m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(server.m_hListenSock, (SOCKADDR*)&server.m_tListenAddr, sizeof(server.m_tListenAddr)))
	{
		clog.Log("ERROR", "ServerInit bind fail");
		return false;
	}

	if (SOCKET_ERROR == listen(server.m_hListenSock, SOMAXCONN))
	{
		clog.Log("ERROR", "ServerInit listen fail");
		return false;
	}

	return true;
}

void CServer::ReadRecv(const PACKETTYPE& type)
{
	if (PACKETTYPE::PT_BallPlaceRecv == type)
	{
		clog.Log("INFO", "PT_BallPlaceRecv recv");
		std::cout << "PT_BallPlaceRecv recv\n";
	}
	else if (PACKETTYPE::PT_ShotDataRecv == type)
	{
		clog.Log("INFO", "PT_ShotDataRecv recv");
		std::cout << "PT_ShotDataRecv recv\n";
	}
	else if (PACKETTYPE::PT_ActiveStateRecv == type)
	{
		clog.Log("INFO", "PT_ActiveStateRecv recv");
		std::cout << "PT_ActiveStateRecv recv\n";
	}
	else
	{
		clog.Log("WARNING", "ReadRecv unknown type");
		std::cout << "ReadRecv unknown type\n";
	}
}

int CServer::SendRecv(const PACKETTYPE& recvtype)
{
	auto& server = CServer::Instance();
	Packet sendrecvpt(recvtype);

	sendrecvpt.SetRecvData();
	return server.ServerSend(sendrecvpt);
}

int CServer::ReadData(Packet& packet)
{
	auto& server = CServer::Instance();
	PACKETTYPE recvtype = PACKETTYPE::PT_None;

	packet.SetRecvData();
	if (SOCKET_ERROR == server.ServerRecv(packet.GetData(), packet.GetSize()))
	{
		clog.Log("ERROR", "ReadData ServerRecv");
		std::cout << "ReadData ServerRecv\n";
		return SOCKET_ERROR;
	}
	else
	{
		if (PACKETTYPE::PT_ClubSetting == packet.GetType())
		{
			clog.Log("INFO", "PT_ClubSetting recv");
			std::cout << "PT_ClubSetting recv\n";
			server.SetClubSetting(packet.GetData());
			recvtype = PACKETTYPE::PT_ClubSettingRecv;
		}
		else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
		{
			clog.Log("INFO", "PT_TeeSetting recv");
			std::cout << "PT_TeeSetting recv\n";
			server.SetTeeSetting(packet.GetData());
			recvtype = PACKETTYPE::PT_TeeSettingRecv;
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "PT_BallPlace recv");
			std::cout << "PT_BallPlace Recv\n";
			server.SetActiveState(packet.GetData());
			recvtype = PACKETTYPE::PT_ActiveStateRecv;
		}
		else
		{
			clog.Log("WARNING", "ReadData unknown type");
			std::cout << "ReadData unknown type\n";
		}
	}
	packet.DeleteData();

	return server.SendRecv(recvtype);
}

int CServer::InputKey(const char input)
{
	auto& server = CServer::Instance();
	Packet pt{};
	if ('w' == input)		//공위치 전달(enum)
	{
		pt.SetSendData(PACKETTYPE::PT_BallPlace, server.GetBallPlace());

		clog.Log("INFO", "PT_BallPlace send");
		std::cout << "PT_BallPlace send\n";
	}
	else if ('e' == input)		//샷정보 전달
	{
		pt.SetSendData(PACKETTYPE::PT_ShotData, server.GetShotData());

		clog.Log("INFO", "PT_ShotData send");
		std::cout << "PT_ShotData send\n";
	}
	else if ('r' == input)		//샷 이후 activestate false 전달
	{
		server.SetActiveState(false);

		pt.SetSendData(PACKETTYPE::PT_ActiveState, server.GetActiveState());

		clog.Log("INFO", "PT_ActiveState(false) send");
		std::cout << "PT_ActiveState(false) send\n";
	}
	else
	{
	}
	return server.ServerSend(pt);
}

DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n" << std::endl;

	while (true)
	{
		if (true == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		{
			if (SOCKET_ERROR == server.InputKey(_getch()))
			{
				clog.Log("ERROR", "SendThread InputKey error");
				std::cout << "SendThread InputKey error\n";
				//err_quit("send()");
				break;
			}
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

	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n" << std::endl;

	while (true)
	{
		Packet pt{};
		if (SOCKET_ERROR == server.ServerRecv(&pt, sizeof(pt)))
		{
			clog.Log("ERROR", "Server_Recv error");
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		else    //에러가 아니라면 데이터 읽기
		{
			if (sizeof(Packet) == pt.GetSize())
			{
				server.ReadRecv(pt.GetType());
			}
			else
			{
				if (SOCKET_ERROR == server.ReadData(pt))
				{
					clog.Log("ERROR", "Server_Recv ReadData error");
					std::cout << "Server_Recv ReadData error\n";
					break;
				}
			}
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

		/*char ipAddress[MAX_PATH] = { 0, };
		sprintf_s(ipAddress, MAX_PATHNAME_LEN, "%s%s", "[login]Client IP : ", inet_ntoa(tCIntAddr.sin_addr));
		clog.Log("INFO", ipAddress);*/

		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept 성공 시

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv 스레드 생성
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		//sprintf_s(ipAddress, MAX_PATHNAME_LEN, "%s%s", "[logout]Client IP : ", inet_ntoa(tCIntAddr.sin_addr));
		//clog.Log("INFO", ipAddress);
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
	}
	closesocket(server.m_hClient);
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


