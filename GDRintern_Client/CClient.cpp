#include "CClient.h"
#include "windows.h"
#include "conio.h"

//로그
CLog clog;

CClient::CClient()
{
	DataInit();
}

CClient::~CClient()
{
	closesocket(Client.m_hSock);
	CloseHandle(Client.m_hSend);
	CloseHandle(Client.m_hRecv);
	WSACleanup();
}

//데이터 초기화
void CClient::DataInit()
{
	this->m_eTee = TEESETTING::T40;

	this->m_eClub = CLUBSETTING::WOOD;

	this->m_eBallPlace = BALLPLACE::OB;

	this->m_bActiveState = false;

	this->m_sdShotData = ShotData{};
}

//통신관련 초기화
bool CClient::ClientInit()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &Client.m_wsaData))
	{
		clog.Log("ERROR", "ClientInit WSAStartup fail");
		std::cout << "ClientInit WSAStartup fail\n";
		return false;
	}

	Client.m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Client.m_hSock)
	{
		clog.Log("ERROR", "ClientInit ListenSocket fail");
		std::cout << "ClientInit ListenSocket fail\n";
		return false;
	}

	Client.m_tAddr.sin_family = AF_INET;
	Client.m_tAddr.sin_port = htons(PORT);
	Client.m_tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	return true;
}

//서버연결
void CClient::ClientConnect()
{
	int retval{ connect(Client.m_hSock, (SOCKADDR*)&Client.m_tAddr, sizeof(Client.m_tAddr)) };
	if (retval == SOCKET_ERROR)
	{
		clog.Log("ERROR", "ClientConnect connect error");
		std::cout << "ClientConnect connect error" << std::endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		std::cout << "MainThread 시작\n";

		Client.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Client.SendThread, (LPVOID)Client.m_hSock, 0, &dwSendThreadID);
		Client.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Client.RecvThread, (LPVOID)Client.m_hSock, 0, &dwRecvThreadID);

		WaitForSingleObject(Client.m_hRecv, INFINITE);
	}
	closesocket(Client.m_hSock);

	return;
}

//send 스레드
DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		Client.InputKey();

		for (auto p = Client.m_qPacket.front(); true != Client.m_qPacket.empty(); )
		{
			p = Client.m_qPacket.front();
			if (SOCKET_ERROR == Client.ClientSend(p))
			{
				clog.Log("ERROR", "SendThread ClientSend SOCKET_ERROR");
				std::cout << "SendThread ClientSend SOCKET_ERROR\n";
				break;
			}
			clog.MakeMsg("INFO", "Send %s", to_string(p->GetType()));
			std::cout << "Send : " << to_string(p->GetType()) << "\n";
			delete p;
			Client.m_qPacket.pop();
		}

	}
	return NULL;
}

//class P : 전송하고자 하는 Packet 또는 Packet하위클래스
//PACKETDATA : Packet인 경우 PACKETTYPE // Packet하위클래스인 경우 전송하고자 하는 데이터
template <class P, class PACKETDATA>
void CClient::SendPacket(PACKETDATA data)
{
	m_qPacket.push(new P(data));
}

//테스트 동작용 키입력(q:ClubSetting, w:TeeSetting, e:active(true), r:active(false))
void CClient::InputKey()
{
	if (true == _kbhit())
	{
		char input = _getch();
		if ('q' == input)		//Club 세팅 전송
		{
			Client.SendPacket<PacketClubSetting>(Client.GetClubSetting());
		}
		else if ('w' == input)		//Tee 세팅 전송
		{
			Client.SendPacket<PacketTeeSetting>(Client.GetTeeSetting());
		}
		else if ('e' == input)		//Active 상태 (모바일->PC 샷 가능 상태 전달)
		{
			Client.SetActiveState(true);
			Client.SendPacket<PacketActiveState>(Client.GetActiveState());
		}
		else if ('r' == input)		//Inactive 상태 (모바일->PC 샷 불가능 상태 전달)
		{
			Client.SetActiveState(false);
			Client.SendPacket<PacketActiveState>(Client.GetActiveState());
		}
	}
}

//recv 스레드
DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	Packet packet{};
	while (true)
	{
		ZeroMemory(&packet, sizeof(Packet));

		if (SOCKET_ERROR == Client.ClientRecv(&packet, PACKETHEADER))
		{
			clog.Log("ERROR", "RecvThread ClientRecv error");
			std::cout << "RecvThread ClientRecv error\n";
			break;
		}
		else	//에러가 아니라면 데이터 읽기
		{
			clog.MakeMsg("INFO", "Recv : %s", to_string(packet.GetType()));
			std::cout << "Recv : " << to_string(packet.GetType()) << "\n";

			if (PACKETHEADER != packet.GetSize())
			{
				ResumeThread(Client.m_hSend);
				if (SOCKET_ERROR == Client.ReadAddData(packet))
				{
					break;
				}
			}
		}
	}
	return NULL;
}

//추가 데이터 recv 시
int CClient::ReadAddData(Packet& packet)
{
	int retval{ 0 };

	unsigned int recvsize{ packet.GetSize() - PACKETHEADER };
	char* recvdata = (char*)malloc(recvsize);
	if (SOCKET_ERROR == Client.ClientRecv(recvdata, recvsize))
	{
		clog.Log("ERROR", "ReadAddData ClientRecv SOCKET_ERROR");
		std::cout << "ReadAddData ClientRecv SOCKET_ERROR\n";
	}
	else
	{
		if (PACKETTYPE::PT_BallPlace == packet.GetType())
		{
			Client.SetBallPlace(recvdata);
			clog.MakeMsg("INFO", "BallPalce : %s", to_string(Client.GetBallPlace()));
			std::cout << "BallPlace : " << Client.GetBallPlace() << "\n";

			Client.SendPacket<Packet>(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			Client.SetShotData(recvdata);
			ShotData sd = Client.GetShotData();
			std::cout << "ShotData : " << sd << "\n";
			clog.MakeMsg("INFO", "[phase %d] : ballspeed[%f], launchangle[%f], "
				"launchdirection[%f], headspeed[%f], backspin[%d], sidespin[%d]",
				sd.phase, sd.ballspeed, sd.launchangle, sd.launchdirection,
				sd.headspeed, sd.backspin, sd.sidespin);

			Client.SendPacket<Packet>(PACKETTYPE::PT_ShotDataRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			Client.SetActiveState(recvdata);
			clog.MakeMsg("INFO", "ActiveState : %s", to_string(Client.GetActiveState()));
			std::cout << "ActiveState : " << Client.GetActiveState() << "\n";

			Client.SendPacket<Packet>(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "ReadAddData Recv unknown type");
			std::cout << "ReadAddData Recv unknown type\n";
		}
	}
	free(recvdata);

	return retval;
}

//recv
int CClient::ClientRecv(void* buf, const int len)
{
	return recv(Client.m_hSock, (char*)buf, len, 0);
}

//send
int CClient::ClientSend(Packet* packet)
{
	int retval{ 0 };
	int sendsize = packet->GetSize();
	char* senddata = nullptr;

	senddata = (char*)malloc(sendsize);
	memcpy_s(senddata, sendsize, packet, sendsize);

	retval = send(Client.m_hSock, senddata, sendsize, 0);

	free(senddata);
	return retval;
}