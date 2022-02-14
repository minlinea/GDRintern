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
		Client.InputKey(_getch());

		if (true != Client.m_qPacket.empty())
		{
			for (auto p = Client.m_qPacket.front(); true != Client.m_qPacket.empty(); )
			{
				p = Client.m_qPacket.front();

				std::cout << p->GetSize() << "\t" << (unsigned int)p->GetType() << "\n";

				if (SOCKET_ERROR == Client.ClientSend(p))
				{
					clog.Log("ERROR", "SendThread ClientSend SOCKET_ERROR");
					std::cout << "SendThread ClientSend SOCKET_ERROR\n";
					break;
				}
				delete p;
				Client.m_qPacket.pop();
			}

		}
		
	}
	return NULL;
}

//Club 세팅 전송
void CClient::SendClubSetting()
{
	m_qPacket.push(new Packet_ClubSetting(Client.GetClubSetting()));

	clog.Log("INFO", "Send PT_ClubSetting");
	std::cout << "Send PT_ClubSetting\n";
}

//Tee 세팅 전송
void CClient::SendTeeSetting()
{
	m_qPacket.push(new Packet_TeeSetting(Client.GetTeeSetting()));

	clog.Log("INFO", "Send PT_TeeSetting");
	std::cout << "Send PT_TeeSetting\n";
}

//샷 가능 여부 전송 (샷 이후 센서 inactive 상황)
void CClient::SendActiveState()
{
	m_qPacket.push(new Packet_ActiveState(Client.GetActiveState()));

	clog.Log("INFO", "Send PT_ActiveState");
	std::cout << "Send PT_ActiveState\n";
}

void CClient::SendNoneAddData(PACKETTYPE type)
{
	m_qPacket.push(new Packet(type));

	//타입관련 입출력 추가 필요
	//clog.Log("INFO", "Send PT_ActiveState");
	//std::cout << "Send PT_ActiveState\n";
}

//테스트 동작용 키입력(q:ClubSetting, w:TeeSetting, e:active(true), r:active(false))
void CClient::InputKey(const char input)
{
	if ('q' == input)		//Club 세팅 전송
	{
		Client.SendClubSetting();
	}
	else if ('w' == input)		//Tee 세팅 전송
	{
		Client.SendTeeSetting();
	}
	else if ('e' == input)		//Active 상태 (모바일->PC 샷 가능 상태 전달)
	{
		Client.SetActiveState(true);
		Client.SendActiveState();
	}
	else if ('r' == input)		//Inactive 상태 (모바일->PC 샷 불가능 상태 전달)
	{
		Client.SetActiveState(false);
		Client.SendActiveState();
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
			if (PACKETHEADER == packet.GetSize())
			{
				Client.ReadHeader(packet.GetType());
			}
			else
			{
				ResumeThread(Client.m_hSend);
				if (SOCKET_ERROR == Client.ReadAddData(packet))
				{
					break;
				}
				else
				{
				}
			}
		}
	}
	return NULL;
}

//추가 데이터 없이 header만 받는 경우
void CClient::ReadHeader(const PACKETTYPE& type)
{
	if (PACKETTYPE::PT_ClubSettingRecv == type)
	{
		clog.Log("INFO", "Recv PT_ClubSettingRecv");
		std::cout << "Recv PT_ClubSettingRecv\n";
	}
	else if (PACKETTYPE::PT_TeeSettingRecv == type)
	{
		clog.Log("INFO", "Recv PT_TeeSettingRecv");
		std::cout << "Recv PT_TeeSettingRecv\n";
	}
	else if (PACKETTYPE::PT_ActiveStateRecv == type)
	{
		clog.Log("INFO", "Recv PT_ActiveStateRecv");
		std::cout << "Recv PT_ActiveStateRecv\n";
	}
	else if (PACKETTYPE::PT_ConnectCheck == type)
	{
		clog.Log("INFO", "Recv PT_ConnectCheck");
		std::cout << "Recv PT_ConnectCheck\n";
	}
	else
	{
		clog.Log("WARNING", "ReadHeader Recv unknown type");
		std::cout << "ReadHeader Recv unknown type\n";
	}
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
			clog.Log("INFO", "Recv PT_BallPlace");

			Client.SetBallPlace(recvdata);
			clog.Log("INFO", to_string(Client.GetBallPlace()));
			std::cout << "Recv PT_BallPlace // " << Client.GetBallPlace() << "\n";

			Client.SendNoneAddData(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ShotData");

			Client.SetShotData(recvdata);
			ShotData sd = Client.GetShotData();
			std::cout << "Recv PT_ShotData // " << sd << "\n";
			clog.MakeMsg("[phase%d] : ballspeed[%f], launchangle[%f]"
				"launchdirection[%f] headspeed[%f] backspin[%d] sidespin[%d]",
				sd.phase, sd.ballspeed, sd.launchangle, sd.launchdirection,
				sd.headspeed, sd.backspin, sd.sidespin);

			Client.SendNoneAddData(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ActiveState");

			Client.SetActiveState(recvdata);
			std::cout << "Recv PT_ActiveState  // " << Client.GetActiveState() << "\n";

			Client.SendNoneAddData(PACKETTYPE::PT_BallPlaceRecv);
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