#include "CClient.h"
#include "windows.h"
#include "conio.h"

CLog clog;

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
		clog.Log("ERROR", "ClientInit WSAStartup fail");
		std::cout << "ClientInit WSAStartup fail\n";
		return false;
	}

	client.m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == client.m_hSock)
	{
		clog.Log("ERROR", "ClientInit ListenSocket fail");
		std::cout << "ClientInit ListenSocket fail\n";
		return false;
	}

	client.m_tAddr.sin_family = AF_INET;
	client.m_tAddr.sin_port = htons(PORT);
	client.m_tAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
	return true;
}

int CClient::InputKey(const char input)
{
	auto& client = CClient::Instance();
	Packet pt{};

	if ('q' == input)		//Club ���� ����
	{
		pt.SetData(PACKETTYPE::PT_ClubSetting, client.GetClubSetting());

		clog.Log("INFO", "Send PT_Setting");
		std::cout << "Send PT_Setting\n";
	}
	else if ('w' == input)		//Tee ���� ����
	{
		pt.SetData(PACKETTYPE::PT_TeeSetting, client.GetTeeSetting());

		clog.Log("INFO", "Send PT_TeeSetting");
		std::cout << "Send PT_TeeSetting\n";
	}
	else if ('e' == input)		//Active ���� (�����->PC �� ���� ���� ����)
	{
		client.SetActiveState(true);

		pt.SetData(PACKETTYPE::PT_ActiveState, client.GetActiveState());

		clog.Log("INFO", "Send PT_Active(true)");
		std::cout << "Send PT_Active(true)\n";
	}
	else if ('r' == input)		//Inactive ���� (�����->PC �� �Ұ��� ���� ����)
	{
		client.SetActiveState(false);

		pt.SetData(PACKETTYPE::PT_ActiveState, client.GetActiveState());

		clog.Log("INFO", "Send PT_Active(false)");
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
	
	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		if (true == _kbhit())		//��Ŷ �׽�Ʈ�� ���� ��ǲ Ű �Է�
		{
			if (SOCKET_ERROR == client.InputKey(_getch()))
			{
				clog.Log("ERROR", "SendThread InputKey error");

				std::cout << "SendThread InputKey error\n";
				break;
			}
		}
		else	//���޻��� �߰�
		{
			
		}

	}
	return NULL;
}

void CClient::ReadRecv(const PACKETTYPE& type)
{
	if (PACKETTYPE::PT_ClubSettingRecv == type)
	{
		clog.Log("INFO", "PT_ClubSettingRecv recv");
		std::cout << "PT_ClubSettingRecv recv\n";
	}
	else if (PACKETTYPE::PT_TeeSettingRecv == type)
	{
		clog.Log("INFO", "PT_TeeSettingRecv recv");
		std::cout << "PT_TeeSettingRecv recv\n";
	}
	else if (PACKETTYPE::PT_ActiveStateRecv == type)
	{
		clog.Log("INFO", "PT_ActiveStateRecv recv");
		std::cout << "PT_ActiveStateRecv recv\n";
	}
	else if (PACKETTYPE::PT_ConnectCheck == type)
	{
		clog.Log("INFO", "PT_Connect recv");
		std::cout << "PT_Connect recv\n";
	}
	else
	{
		clog.Log("WARNING", "ReadRecv unknown type");
		std::cout << "ReadRecv unknown type\n";
	}

}

int CClient::SendRecv(const PACKETTYPE& sendtype)
{
	auto& client = CClient::Instance();
	Packet sendpt(sendtype);
	sendpt.SetData();
	return client.ClientSend(sendpt);
}

int CClient::ReadData(Packet& packet)
{
	auto& client = CClient::Instance();
	Packet recvpt{};

	packet.SetData();
	if (SOCKET_ERROR == client.ClientRecv(packet.GetData(), packet.GetSize()))
	{
		clog.Log("ERROR", "ReadAddData ClientRecv");
		std::cout << "ReadAddData ClientRecv\n";
	}
	else
	{
		if (PACKETTYPE::PT_BallPlace == packet.GetType())
		{
			clog.Log("INFO", "PT_BallPlace Recv");
			std::cout << "PT_BallPlace Recv\n";

			client.SetBallPlace(packet.GetData());
			recvpt.SetType(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			clog.Log("INFO", "PT_ShotData Recv");
			std::cout << "PT_ShotData Recv\n";

			client.SetShotData(packet.GetData());
			recvpt.SetType(PACKETTYPE::PT_ShotDataRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "PT_ActiveState Recv");
			std::cout << "PT_ShotData Recv\n";

			client.SetActiveState(packet.GetData());
			recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "ReadAddData unknown type");
			std::cout << "ReadAddData unknown type\n";
		}
	}

	recvpt.SetData();
	return client.ClientSend(recvpt);
}

DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	auto& client = CClient::Instance();

	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	Packet pt;
	while (true)
	{
		ZeroMemory(&pt, sizeof(Packet));
		if (SOCKET_ERROR == client.ClientRecv((char*)&pt, sizeof(Packet)))
		{
			clog.Log("ERROR", "RecvThread ClientRecv error");
			std::cout << "RecvThread ClientRecv error\n";
			break;
		}
		else	//������ �ƴ϶�� ������ �б�
		{
			if (sizeof(Packet) == pt.GetSize())
			{
				client.ReadRecv(pt.GetType());
			}
			else
			{
				if (SOCKET_ERROR == client.ReadData(pt))
				{
					clog.Log("ERROR", "ReadAddData error");
					std::cout << "ReadAddData error\n";
					break;
				}
				pt.DeleteData();
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
		clog.Log("ERROR", "connect error");
		std::cout << "connect error" << std::endl;
	}
	else
	{
		DWORD dwSendThreadID, dwRecvThreadID;

		std::cout << "MainThread ����\n";

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


