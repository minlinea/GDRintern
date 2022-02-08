#include "CClient.h"
#include "windows.h"
#include "conio.h"

//�α�
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

//������ �ʱ�ȭ
void CClient::DataInit()
{
	m_eTee = TEESETTING::T40;

	m_eClub = CLUBSETTING::DRIVER;

	m_eBallPlace = BALLPLACE::OB;

	m_bActiveState = false;

	m_sdShotData = ShotData{};
}

//��Ű��� �ʱ�ȭ
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

//��������
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

//send ������
DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	auto& client = CClient::Instance();

	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		if (1 == _kbhit())		//��Ŷ �׽�Ʈ�� ���� ��ǲ Ű �Է�
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

//�׽�Ʈ ���ۿ� Ű�Է�(q:ClubSetting, w:TeeSetting, e:active(true), r:active(false))
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
		pt.SetData();
	}

	return client.ClientSend(pt);
}

//recv ������
DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	auto& client = CClient::Instance();

	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	while (true)
	{
		Packet packet{};

		if (SOCKET_ERROR == client.ClientRecv(&packet, sizeof(packet)))
		{
			clog.Log("ERROR", "RecvThread ClientRecv error");
			std::cout << "RecvThread ClientRecv error\n";
			break;
		}
		else	//������ �ƴ϶�� ������ �б�
		{
			if (PACKETHEADER == packet.GetSize())
			{
				client.ReadHeader(packet.GetType());
			}
			else
			{
				if (SOCKET_ERROR == client.ReadAddData(packet))
				{
					clog.Log("ERROR", "RecvThread ReadAddData error");
					std::cout << "RecvThread ReadAddData error\n";
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

//�߰� ������ ���� header�� �޴� ���
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
		clog.Log("WARNING", "Recv ReadHeader unknown type");
		std::cout << "Recv ReadHeader unknown type\n";
	}

}

//�߰� ������ recv ��
int CClient::ReadAddData(Packet& packet)
{
	auto& client = CClient::Instance();
	Packet recvpt{};

	packet.SetData();
	if (SOCKET_ERROR == client.ClientRecv(packet.GetData(), packet.GetSize()))
	{
		clog.Log("ERROR", "ReadAddData SOCKET_ERROR");
		std::cout << "ReadAddData SOCKET_ERROR\n";
	}
	else
	{
		if (PACKETTYPE::PT_BallPlace == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_BallPlace");
			client.SetBallPlace(packet.GetData());
			clog.Log("INFO", to_string(client.GetBallPlace()));
			std::cout << "Recv PT_BallPlace // " << client.GetBallPlace() << "\n";

			recvpt.SetType(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ShotData");

			client.SetShotData(packet.GetData());
			ShotData sd = client.GetShotData();

			std::cout << "Recv PT_ShotData // " << sd << "\n";
			clog.MakeMsg("[phase%d] : ballspeed[%f], launchangle[%f]"
				"launchdirection[%f] headspeed[%f] backspin[%d] sidespin[%d]",
				sd.phase, sd.ballspeed, sd.launchangle, sd.launchdirection,
				sd.headspeed, sd.backspin, sd.sidespin);

			recvpt.SetType(PACKETTYPE::PT_ShotDataRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ActiveState");
			client.SetActiveState(packet.GetData());
			std::cout << "Recv PT_ActiveState  // " << client.GetActiveState() << "\n";

			recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "Recv ReadAddData unknown type");
			std::cout << "Recv ReadAddData unknown type\n";
		}
	}

	recvpt.SetData();
	return client.ClientSend(recvpt);
}

//send
int CClient::ClientSend(Packet& packet)
{
	auto& client = CClient::Instance();
	return send(client.m_hSock, (const char*)packet.GetData(), packet.GetSize(), 0);
}

//recv
int CClient::ClientRecv(void* buf, const int len)
{
	auto& client = CClient::Instance();
	return recv(client.m_hSock, (char*)buf, len, 0);
}