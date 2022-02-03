#include "CClient.h"
#include "windows.h"
#include "conio.h"

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
	m_eTee = TEESETTING::T40;
	m_eClub = CLUBSETTING::DRIVER;

	m_eBallPlace = BALLPLACE::OB;

	m_bActiveState = false;

	m_sdShotData = ShotData{};
}

bool CClient::ClientInit()
{
	auto& client = CClient::Instance();
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

int CClient::InputKey(const char input)
{
	auto& client = CClient::Instance();
	int retval{ 1 };
	Packet pt{};

	if ('q' == input)		//Club 세팅 전송
	{
		pt.SetType(PACKETTYPE::PT_ClubSetting);
		pt.SetSize(sizeof(CLUBSETTING));
		pt.SetSendData(client.GetClubSetting());

		std::cout << "Send PT_Setting\n";
	}
	else if ('w' == input)		//Tee 세팅 전송
	{
		pt.SetType(PACKETTYPE::PT_TeeSetting);
		pt.SetSize(sizeof(TEESETTING));
		pt.SetSendData(client.GetTeeSetting());

		std::cout << "Send PT_TeeSetting\n";
	}
	else if ('e' == input)		//Active 상태 (모바일->PC 샷 가능 상태 전달)
	{
		client.SetActiveState(true);

		pt.SetType(PACKETTYPE::PT_ActiveState);
		pt.SetSize(sizeof(ACTIVESTATE));
		pt.SetSendData(client.GetActiveState());

		std::cout << "Send PT_Active(true)\n";
	}
	else if ('r' == input)		//Inactive 상태 (모바일->PC 샷 불가능 상태 전달)
	{
		client.SetActiveState(false);

		pt.SetType(PACKETTYPE::PT_ActiveState);
		pt.SetSize(sizeof(ACTIVESTATE));
		pt.SetSendData(client.GetActiveState());

		std::cout << "Send PT_Active(false)\n";
	}
	else
	{
	}

	return client.ClientSend(pt);
}

DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	auto& client = CClient::Instance();
	int retval{ 0 };
	while (true)
	{
		if (true == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		{
			if (SOCKET_ERROR == client.InputKey(_getch()))
			{
				std::cout << "SendThread InputKey error\n";
				//err_quit("send()");
				break;
			}//InputKey->ClientSend
		}
		else	//유휴상태 추가
		{
			
		}

	}
	return NULL;
}

void CClient::ReadRecv(const PACKETTYPE& type)
{
	if (PACKETTYPE::PT_ClubSettingRecv == type)
	{
		std::cout << "PT_ClubSettingRecv recv\n";
	}
	else if (PACKETTYPE::PT_TeeSettingRecv == type)
	{
		std::cout << "PT_TeeSettingRecv recv\n";
	}
	else if (PACKETTYPE::PT_ActiveStateRecv == type)
	{
		std::cout << "PT_ActiveStateRecv recv\n";
	}
	else
	{
		std::cout << "ReadRecv unknown type\n";
	}

}

int CClient::SendRecv(const PACKETTYPE& recvtype)
{
	auto& client = CClient::Instance();
	Packet sendrecvpt(recvtype);
	sendrecvpt.SetRecvData();
	return client.ClientSend(sendrecvpt);
}

int CClient::ReadData(Packet& packet)
{
	auto& client = CClient::Instance();
	PACKETTYPE recvtype = PACKETTYPE::PT_None;

	packet.SetRecvData();
	if (SOCKET_ERROR == client.ClientRecv(packet.GetData(), packet.GetSize()))
	{
		std::cout << "ReadData ClientRecv\n";
	}
	else
	{
		if (PACKETTYPE::PT_BallPlace == packet.GetType())
		{
			std::cout << "PT_BallPlace Recv\n";
			client.SetBallPlace(packet.GetData());
			recvtype = PACKETTYPE::PT_BallPlaceRecv;
		}

		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			std::cout << "PT_ShotData Recv\n";
			client.SetShotData(packet.GetData());
			recvtype = PACKETTYPE::PT_ShotDataRecv;
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			std::cout << "PT_ActiveState recv\n";
			client.SetActiveState(packet.GetData());
			recvtype = PACKETTYPE::PT_ActiveStateRecv;
		}
		else
		{
			std::cout << "ReadData unknown type\n";
		}
	}
	packet.DeleteData();

	return client.SendRecv(recvtype);
}

DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	auto& client = CClient::Instance();

	while (true)
	{
		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == client.ClientRecv((char*)&pt, sizeof(Packet)))
		{
			std::cout << "RecvThread ClientRecv error\n";
			//err_quit("recv()");
			break;
		}
		else	//에러가 아니라면 데이터 읽기
		{
			if (sizeof(Packet) == pt.GetSize())
			{
				client.ReadRecv(pt.GetType());
			}
			else
			{
				if (SOCKET_ERROR == client.ReadData(pt))
				{
					std::cout << "ReadData error\n";
					//err_quit("recv()");
					break;
				}
			}
			
		}
		
	}
	return NULL;
}

void CClient::ClientConnect()
{
	auto& client = CClient::Instance();
	int retval{ connect(client.m_hSock, (SOCKADDR*)&client.m_tAddr, sizeof(client.m_tAddr)) };
	if (retval == SOCKET_ERROR)
	{
		std::cout << "connect error" << std::endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		std::cout << "MainThread 시작\n";

		m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client.SendThread, (LPVOID)client.m_hSock, 0, &dwSendThreadID);
		m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)client.RecvThread, (LPVOID)client.m_hSock, 0, &dwRecvThreadID);

		WaitForSingleObject(m_hRecv, INFINITE);
	}
	closesocket(m_hSock);

	return;
}

int CClient::ClientSend(Packet& packet)
{
	auto& client = CClient::Instance();
	return send(client.m_hSock, (const char*)packet.GetData(), packet.GetSize(), 0);
}

int CClient::ClientRecv(void* buf, const int len)
{
	auto& client = CClient::Instance();
	return recv(client.m_hSock, (char*)buf, len, 0);
}


