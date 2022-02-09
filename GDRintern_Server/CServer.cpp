#include "CServer.h"
#include "conio.h"

//로그
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

//데이터 초기화
void CServer::DataInit()
{
	m_eTee = TEESETTING::T40;

	m_eClub = CLUBSETTING::DRIVER;

	m_eBallPlace = BALLPLACE::OB;

	m_bActiveState = false;

	m_sdShotData = ShotData{ 0,1,2,3,4,5,6 };

	time(&m_tNowTime);
	m_tBeforeTime = m_tNowTime;
	m_iWaitingCount = 0;
}

//통신관련 초기화
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

//서버 시작
void CServer::ServerAccept()
{
	auto& server = CServer::Instance();
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		server.m_hClient = accept(server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);

		clog.Log("LOGIN", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept 성공 시

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv 스레드 생성
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		clog.Log("LOGOUT", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
	}
	closesocket(server.m_hClient);
	return;
}

//send 스레드
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{

		Packet packet{ PACKETTYPE::PT_ConnectCheck };

		time(&server.m_tNowTime);
		if (1 == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		{
			if (SOCKET_ERROR == server.InputKey(_getch()))
			{
				clog.Log("ERROR", "SendThread InputKey error");
				std::cout << "SendThread InputKey error\n";
				break;
			}
			else
			{
			}
			server.m_tBeforeTime = server.m_tNowTime;
			server.m_iWaitingCount = 0;
		}
		else
		{
			if (WaitingTime <= server.m_tNowTime - server.m_tBeforeTime)
			{
				server.ServerSend(&packet);

				server.m_tBeforeTime = server.m_tNowTime;

				clog.Log("INFO", "PT_ConnectCheck");
				std::cout << "Send PT_ConnectCheck\n";

				++server.m_iWaitingCount;
				if (MAXWaitingCount <= server.m_iWaitingCount)
				{
					SuspendThread(server.m_hSend);
				}
				else
				{
				}
			}
			else
			{
			}
		}
	}
	return NULL;
}

//테스트 동작용 키입력(w:ballplace, e:shotdata, r:active(false)
int CServer::InputKey(const char input)
{
	auto& server = CServer::Instance();

	Packet* packet{ nullptr };
	char* senddata{ nullptr };
	int retval{ 0 };

	if ('w' == input)		//공위치 전달(enum)
	{
		packet = new Packet_BallPlace(server.GetBallPlace());

		clog.Log("INFO", "Send PT_BallPlace");
		std::cout << "Send PT_BallPlace\n";
	}
	else if ('e' == input)		//샷정보 전달
	{
		packet = new Packet_ShotData(server.GetShotData());

		clog.Log("INFO", "Send PT_ShotData");
		std::cout << "Send PT_ShotData\n";
	}
	else if ('r' == input)		//샷 이후 activestate false 전달
	{
		server.SetActiveState(false);

		packet = new Packet_ActiveState(server.GetActiveState());

		clog.Log("INFO", "Send PT_ActiveState(false)");
		std::cout << "Send PT_ActiveState(false)\n";
	}
	else
	{
		return retval;
	}

	if (packet != nullptr)
	{
		retval = server.ServerSend(packet);
		delete packet;
	}

	return retval;
}

//recv 스레드
DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	while (true)
	{
		Packet packet{};

		if (SOCKET_ERROR == server.ServerRecv(&packet, sizeof(packet)))
		{
			clog.Log("ERROR", "RecvThread ServerRecv error");
			std::cout << "RecvThread ServerRecv error\n";
			break;
		}
		else    //에러가 아니라면 데이터 읽기
		{
			server.m_iWaitingCount = 0;
			ResumeThread(server.m_hSend);

			if (sizeof(Packet) == packet.GetSize())
			{
				server.ReadHeader(packet.GetType());
			}
			else
			{
				if (SOCKET_ERROR == server.ReadAddData(packet))
				{
					clog.Log("ERROR", "ServerRecv ReadAddData error");
					std::cout << "ServerRecv ReadAddData error\n";
					break;
				}
			}
			server.m_tBeforeTime = server.m_tNowTime;
		}

	}
	return NULL;
}

//추가 데이터 없이 header만 받는 경우
void CServer::ReadHeader(const PACKETTYPE& type)
{
	if (PACKETTYPE::PT_BallPlaceRecv == type)
	{
		clog.Log("INFO", "Recv PT_BallPlaceRecv");
		std::cout << "Recv PT_BallPlaceRecv\n";
	}
	else if (PACKETTYPE::PT_ShotDataRecv == type)
	{
		clog.Log("INFO", "Recv PT_ShotDataRecv");
		std::cout << "Recv PT_ShotDataRecv\n";
	}
	else if (PACKETTYPE::PT_ActiveStateRecv == type)
	{
		clog.Log("INFO", "Recv PT_ActiveStateRecv");
		std::cout << "Recv PT_ActiveStateRecv\n";
	}
	else
	{
		clog.Log("WARNING", "Recv ReadHeader unknown type");
		std::cout << "Recv ReadHeader unknown type\n";
	}
}

//추가 데이터 recv 시
int CServer::ReadAddData(Packet& packet)
{
	auto& server = CServer::Instance();
	Packet recvpt{PACKETTYPE::PT_None};
	int retval{ 0 };

	char* recvdata = (char*)malloc(packet.GetSize());
	if (SOCKET_ERROR == server.ServerRecv(recvdata, packet.GetSize()))
	{
		clog.Log("ERROR", "ReadAddData SOCKET_ERROR");
		std::cout << "ReadAddData SOCKET_ERROR\n";
		return SOCKET_ERROR;
	}
	else
	{
		if (PACKETTYPE::PT_ClubSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ClubSetting");
			server.SetClubSetting(recvdata);
			std::cout << "Recv PT_ClubSetting // " << server.GetClubSetting() << "\n";
			recvpt.SetType(PACKETTYPE::PT_ClubSettingRecv);
		}
		else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_TeeSetting");
			server.SetTeeSetting(recvdata);
			std::cout << "Recv PT_TeeSetting // " << server.GetTeeSetting() << "\n";
			recvpt.SetType(PACKETTYPE::PT_TeeSettingRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_BallPlace");
			server.SetActiveState(recvdata);
			std::cout << "Recv PT_BallPlace // " << server.GetBallPlace() << "\n";
			recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "Recv ReadAddData unknown type");
			std::cout << "Recv ReadAddData unknown type\n";
		}
	}

	//정상적인 데이터 recv 시 응답 send 진행
	if (PACKETTYPE::PT_None != recvpt.GetType())
	{
		retval = server.ServerSend(&recvpt);
	}

	free(recvdata);

	return retval;
}

//recv
int CServer::ServerRecv(void* buf, const int len)
{
	auto& server = CServer::Instance();
	return recv(server.m_hClient, (char*)buf, len, 0);
}


//send
int CServer::ServerSend(Packet* packet)
{
	auto& server = CServer::Instance();
	int retval{ 0 };
	int sendsize = packet->GetSize();
	char* senddata = nullptr;

	senddata = (char*)malloc(sendsize);
	memcpy_s(senddata, sendsize, packet, sendsize);

	retval = send(server.m_hClient, senddata, sendsize, 0);
	
	free(senddata);
	return retval;
}