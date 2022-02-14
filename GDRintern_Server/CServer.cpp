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
	closesocket(Server.m_hListenSock);
	CloseHandle(Server.m_hSend);
	CloseHandle(Server.m_hRecv);
	CloseHandle(Server.m_hListen);
	WSACleanup();
}

//데이터 초기화
void CServer::DataInit()
{
	this->m_eTee = TEESETTING::T40;

	this->m_eClub = CLUBSETTING::DRIVER;
	
	this->m_eBallPlace = BALLPLACE::OB;
	
	this->m_bActiveState = false;
	
	this->m_sdShotData = ShotData{ 0,1,2,3,4,5,6 };
	
	this->m_bConnect = true;

	time(&m_tNowTime);
	this->m_tBeforeTime = m_tNowTime;
	this->m_iWaitingCount = 0;
}

//통신관련 초기화
bool CServer::ServerInit()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &Server.m_wsaData))
	{
		Server.PrintLog("ERROR", "ServerInit WSAStartup fail");
		return false;
	}

	Server.m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Server.m_hListenSock)
	{
		Server.PrintLog("ERROR", "ServerInit ListenSocket fail");
		return false;
	}

	Server.m_tListenAddr.sin_family = AF_INET;
	Server.m_tListenAddr.sin_port = htons(PORT);
	Server.m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(Server.m_hListenSock, (SOCKADDR*)&Server.m_tListenAddr, sizeof(Server.m_tListenAddr)))
	{
		Server.PrintLog("ERROR", "ServerInit bind fail");
		clog.Log("ERROR", "ServerInit bind fail");
		return false;
	}

	if (SOCKET_ERROR == listen(Server.m_hListenSock, SOMAXCONN))
	{
		Server.PrintLog("ERROR", "ServerInit listen fail");
		return false;
	}

	return true;
}

//서버 시작
void CServer::ServerAccept()
{
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		Server.m_hClient = accept(Server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);

		Server.PrintLog("LOGIN", "login : %s", inet_ntoa(tCIntAddr.sin_addr));

		Server.m_bConnect = true;
		Server.m_tBeforeTime = Server.m_tNowTime = time(NULL);

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv 스레드 생성
		Server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.RecvThread, (LPVOID)Server.m_hClient, 0, &dwRecvThreadID);
		Server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.SendThread, (LPVOID)Server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(Server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		
		Server.m_bConnect = false;

		TerminateThread(Server.m_hRecv, 0);
		TerminateThread(Server.m_hRecv, 0);

		Server.PrintLog("LOGOUT", "logout : %s", inet_ntoa(tCIntAddr.sin_addr));
	}
	closesocket(Server.m_hClient);
	return;
}

//로그 출력 관련
void CServer::PrintLog(const char* logtype, const char* logmsg, ...)
{
	va_list args;

	va_start(args, logmsg);

	char msg[MAX_MESSAGE_LEN] = { 0, };
	vsnprintf_s(msg, sizeof(msg), MAX_MESSAGE_LEN, logmsg, args);

	va_end(args);

	std::cout << msg << "\n";

	clog.Log(logtype, msg);
}

//send 스레드
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	Server.PrintLog("INFO", "SendThread ON");

	while (true == Server.m_bConnect)
	{
		time(&Server.m_tNowTime);

		Server.InputKey();		//테스트용 키입력 함수

		//대기 시간이 지나도 클라이언트의 별도 입력이 없는 경우
		if (WaitingTime <= Server.m_tNowTime - Server.m_tBeforeTime)	//ConnectCheck 전송
		{
			Server.SendPacket<Packet>(PACKETTYPE::PT_ConnectCheck);
			Server.m_tBeforeTime = Server.m_tNowTime;				//BeforeTime 갱신
			++Server.m_iWaitingCount;								//count 누적
		}

		if (true != Server.m_qPacket.empty())
		{
			for (auto p = Server.m_qPacket.front(); true != Server.m_qPacket.empty(); )
			{
				p = Server.m_qPacket.front();
				if (SOCKET_ERROR == Server.ServerSend(p))
				{
					Server.PrintLog("ERROR", "SendThread ClientSend SOCKET_ERROR");
					Server.m_bConnect = false;
				}
				Server.PrintLog("INFO", "Send : %s", to_string(p->GetType()));
				delete p;
				Server.m_qPacket.pop();
			}


			if (MAXWaitingCount <= Server.m_iWaitingCount)			//일정 count를 넘겼다면
			{
				SuspendThread(Server.m_hSend);						//스레드 일시정지
			}
		}
	}
	return NULL;
}

//class P : 전송하고자 하는 Packet 또는 Packet하위클래스
//PACKETDATA : Packet인 경우 PACKETTYPE // Packet하위클래스인 경우 전송하고자 하는 데이터
template <class P, class PACKETDATA>
void CServer::SendPacket(PACKETDATA data)
{
	m_qPacket.push(new P(data));
}

//테스트 동작용 키입력(w:ballplace, e:shotdata, r:active(false)
void CServer::InputKey()
{
	if (_kbhit())
	{
		char input = _getch();
		if ('w' == input)		//공위치 전달(enum)
		{
			Server.SendPacket<PacketBallPlace>(Server.GetBallPlace());
		}
		else if ('e' == input)		//샷정보 전달
		{
			Server.SendPacket<PacketShotData>(Server.GetShotData());
		}
		else if ('r' == input)		//샷 이후 activestate false 전달
		{
			Server.SendPacket<PacketActiveState>(Server.GetActiveState());
		}
	}
}

//recv 스레드
DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	Server.PrintLog("INFO", "RecvThread ON");

	Packet packet{};
	while (true == Server.m_bConnect)
	{
		ZeroMemory(&packet, sizeof(Packet));

		if (SOCKET_ERROR == Server.ServerRecv(&packet, PACKETHEADER))
		{
			Server.PrintLog("ERROR", "RecvThread ServerRecv SOCKET_ERROR");
			Server.m_bConnect = false;;
		}
		else    //에러가 아니라면 데이터 읽기
		{
			Server.PrintLog("INFO", "Recv : %s", to_string(packet.GetType()));

			//무언가 입력을 받은 경우 대기 카운트 초기화
			Server.m_iWaitingCount = 0;
			ResumeThread(Server.m_hSend);	//SendThread resume
			Server.m_tBeforeTime = Server.m_tNowTime;		//대기시간 갱신

			if (PACKETHEADER != packet.GetSize())
			{
				if (SOCKET_ERROR == Server.ReadAddData(packet))
				{
					Server.PrintLog("ERROR", "RecvThread ReadAddData SOCKET_ERROR");
					Server.m_bConnect = false;
				}
			}
		}
	}
	return NULL;
}

//추가 데이터 recv 시
int CServer::ReadAddData(Packet& packet)
{
	int retval{ 0 };

	unsigned int recvsize{ packet.GetSize() - PACKETHEADER };
	char* recvdata = (char*)malloc(recvsize);
	retval = Server.ServerRecv(recvdata, recvsize);

	if (SOCKET_ERROR != retval)
	{

		if (PACKETTYPE::PT_ClubSetting == packet.GetType())
		{
			Server.SetClubSetting(recvdata);

			Server.PrintLog("INFO", "ClubSetting : %s", to_string(Server.GetClubSetting()));

			Server.SendPacket<Packet>(PACKETTYPE::PT_ClubSettingRecv);
		}
		else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
		{
			Server.SetTeeSetting(recvdata);

			Server.PrintLog("INFO", "TeeSetting : %s", to_string(Server.GetTeeSetting()));

			Server.SendPacket<Packet>(PACKETTYPE::PT_TeeSettingRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			Server.SetActiveState(recvdata);

			Server.PrintLog("INFO", "ActiveState : %s", to_string(Server.GetActiveState()));

			Server.SendPacket<Packet>(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			Server.PrintLog("WARNING", "ReadAddData recv unknown type");
		}
	}
	free(recvdata);

	return retval;
}

//recv
int CServer::ServerRecv(void* buf, const int len)
{
	return recv(Server.m_hClient, (char*)buf, len, 0);
}

//send
int CServer::ServerSend(Packet* packet)
{
	int retval{ 0 };
	int sendsize = packet->GetSize();
	char* senddata = nullptr;

	senddata = (char*)malloc(sendsize);
	memcpy_s(senddata, sendsize, packet, sendsize);

	retval = send(Server.m_hClient, senddata, sendsize, 0);
	
	free(senddata);
	return retval;
}