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
	
	time(&m_tNowTime);
	this->m_tBeforeTime = m_tNowTime;
	this->m_iWaitingCount = 0;
}

//통신관련 초기화
bool CServer::ServerInit()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &Server.m_wsaData))
	{
		clog.Log("ERROR", "ServerInit WSAStartup fail");
		return false;
	}

	Server.m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Server.m_hListenSock)
	{
		clog.Log("ERROR", "ServerInit ListenSocket fail");
		return false;
	}

	Server.m_tListenAddr.sin_family = AF_INET;
	Server.m_tListenAddr.sin_port = htons(PORT);
	Server.m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(Server.m_hListenSock, (SOCKADDR*)&Server.m_tListenAddr, sizeof(Server.m_tListenAddr)))
	{
		clog.Log("ERROR", "ServerInit bind fail");
		return false;
	}

	if (SOCKET_ERROR == listen(Server.m_hListenSock, SOMAXCONN))
	{
		clog.Log("ERROR", "ServerInit listen fail");
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

		clog.Log("LOGIN", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept 성공 시

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv 스레드 생성
		Server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.RecvThread, (LPVOID)Server.m_hClient, 0, &dwRecvThreadID);
		Server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.SendThread, (LPVOID)Server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(Server.m_hSend, INFINITE);//Send스레드 종료 대기(클라이언트와의 연결 종료 여부 확인)
		clog.Log("LOGOUT", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
	}
	closesocket(Server.m_hClient);
	return;
}

//send 스레드
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		time(&Server.m_tNowTime);

		Server.InputKey();		//테스트용 키입력 함수

		//대기 시간이 지나도 클라이언트의 별도 입력이 없는 경우
		if (WaitingTime <= Server.m_tNowTime - Server.m_tBeforeTime)	//ConnectCheck 전송
		{
			Server.SendData<Packet>(PACKETTYPE::PT_ConnectCheck);
			Server.m_tBeforeTime = Server.m_tNowTime;				//BeforeTime 갱신
			++Server.m_iWaitingCount;								//count 누적
		}

		for (auto p = Server.m_qPacket.front(); true != Server.m_qPacket.empty(); )
		{
			p = Server.m_qPacket.front();
			if (SOCKET_ERROR == Server.ServerSend(p))
			{
				clog.Log("ERROR", "SendThread ClientSend SOCKET_ERROR");
				std::cout << "SendThread ClientSend SOCKET_ERROR\n";
				break;
			}
			clog.MakeMsg("INFO", "Send %s", to_string(p->GetType()));
			std::cout << "Send : " << to_string(p->GetType()) << "\n";
			delete p;
			Server.m_qPacket.pop();
		}


		if (MAXWaitingCount <= Server.m_iWaitingCount)			//일정 count를 넘겼다면
		{
			SuspendThread(Server.m_hSend);						//스레드 일시정지
		}
	}
	return NULL;
}

//class P : 전송하고자 하는 Packet 또는 Packet하위클래스
//PACKETDATA : Packet인 경우 PACKETTYPE // Packet하위클래스인 경우 전송하고자 하는 데이터
template <class P, class PACKETDATA>
void CServer::SendData(PACKETDATA data)
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
			Server.SendData<PacketBallPlace>(Server.GetBallPlace());
		}
		else if ('e' == input)		//샷정보 전달
		{
			Server.SendData<PacketShotData>(Server.GetShotData());
		}
		else if ('r' == input)		//샷 이후 activestate false 전달
		{
			Server.SendData<PacketActiveState>(Server.GetActiveState());
		}
	}
}

//recv 스레드
DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	Packet packet{};
	while (true)
	{
		ZeroMemory(&packet, sizeof(Packet));

		if (SOCKET_ERROR == Server.ServerRecv(&packet, PACKETHEADER))
		{
			clog.Log("ERROR", "RecvThread ServerRecv SOCKET_ERROR");
			std::cout << "RecvThread ServerRecv SOCKET_ERROR\n";
			break;
		}
		else    //에러가 아니라면 데이터 읽기
		{
			//무언가 입력을 받은 경우 대기 카운트 초기화
			Server.m_iWaitingCount = 0;
			ResumeThread(Server.m_hSend);	//SendThread resume
			Server.m_tBeforeTime = Server.m_tNowTime;		//대기시간 갱신

			if (PACKETHEADER == packet.GetSize())
			{
				Server.ReadHeader(packet.GetType());
			}
			else
			{
				if (SOCKET_ERROR == Server.ReadAddData(packet))
				{
					clog.Log("ERROR", "ReadAddData ServerRecv SOCKET_ERROR");
					std::cout << "ReadAddData ServerRecv SOCKET_ERROR\n";
					break;
				}
			}
			
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
		clog.Log("WARNING", "ReadHeader Recv unknown type");
		std::cout << "ReadHeader Recv unknown type\n";
	}
}

//추가 데이터 recv 시
int CServer::ReadAddData(Packet& packet)
{
	int retval{ 0 };

	unsigned int recvsize{ packet.GetSize() - PACKETHEADER };
	char* recvdata = (char*)malloc(recvsize);
	retval = Server.ServerRecv(recvdata, recvsize);

	if (SOCKET_ERROR == retval)
	{
		return SOCKET_ERROR;
	}
	else
	{
		if (PACKETTYPE::PT_ClubSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ClubSetting");
			Server.SetClubSetting(recvdata);

			clog.MakeMsg("INFO", "ClubSetting : %s", to_string(Server.GetClubSetting()));
			std::cout << "Recv PT_ClubSetting // " << Server.GetClubSetting() << "\n";
			
			Server.SendData<Packet>(PACKETTYPE::PT_ClubSettingRecv);
		}
		else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_TeeSetting");
			Server.SetTeeSetting(recvdata);

			std::cout << "Recv PT_TeeSetting // " << Server.GetTeeSetting() << "\n";
			clog.MakeMsg("INFO", "TeeSetting : %s", to_string(Server.GetTeeSetting()));

			Server.SendData<Packet>(PACKETTYPE::PT_TeeSettingRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ActiveState");
			Server.SetActiveState(recvdata);

			std::cout << "Recv PT_ActiveState // " << Server.GetActiveState() << "\n";
			clog.MakeMsg("INFO", "ActiveState : %s", to_string(Server.GetActiveState()));

			Server.SendData<Packet>(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "ReadAddData recv unknown type");
			std::cout << "ReadAddData recv unknown type\n";
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