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
	closesocket(Client.m_hSock);
	CloseHandle(Client.m_hSend);
	CloseHandle(Client.m_hRecv);
	WSACleanup();
}

//������ �ʱ�ȭ
void CClient::DataInit()
{
	this->m_eTee = TEESETTING::T40;

	this->m_eClub = CLUBSETTING::DRIVER;

	this->m_eBallPlace = BALLPLACE::OB;

	this->m_bActiveState = false;

	this->m_sdShotData = ShotData{};
}

//��Ű��� �ʱ�ȭ
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

//��������
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

		std::cout << "MainThread ����\n";

		Client.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Client.SendThread, (LPVOID)Client.m_hSock, 0, &dwSendThreadID);
		Client.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Client.RecvThread, (LPVOID)Client.m_hSock, 0, &dwRecvThreadID);

		WaitForSingleObject(Client.m_hRecv, INFINITE);
	}
	closesocket(Client.m_hSock);

	return;
}

//send ������
DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		if (1 == _kbhit())		//��Ŷ �׽�Ʈ�� ���� ��ǲ Ű �Է�
		{
			if (SOCKET_ERROR == Client.InputKey(_getch()))
			{
				break;
			}
		}
		else
		{
		}

	}
	return NULL;
}

//Club ���� ����
int CClient::SendClubSetting()
{
	int retval{ 0 };
	Packet_ClubSetting* packet{ nullptr };

	packet = new Packet_ClubSetting(Client.GetClubSetting());

	clog.Log("INFO", "Send PT_ClubSetting");
	std::cout << "Send PT_ClubSetting\n";

	retval = Client.ClientSend(packet);
	if (SOCKET_ERROR == retval)
	{
		clog.Log("ERROR", "SendClubSetting ClientSend SOCKET_ERROR");
		std::cout << "SendClubSetting ClientSend SOCKET_ERROR\n";
	}
	delete packet;

	return retval;
}

//Tee ���� ����
int CClient::SendTeeSetting()
{
	int retval{ 0 };
	Packet_TeeSetting* packet{ nullptr };

	packet = new Packet_TeeSetting(Client.GetTeeSetting());

	clog.Log("INFO", "Send PT_TeeSetting");
	std::cout << "Send PT_TeeSetting\n";

	retval = Client.ClientSend(packet);
	if (SOCKET_ERROR == retval)
	{
		clog.Log("ERROR", "SendTeeSetting ClientSend SOCKET_ERROR");
		std::cout << "SendTeeSetting ClientSend SOCKET_ERROR\n";
	}
	delete packet;

	return retval;
}

//�� ���� ���� ���� (�� ���� ���� inactive ��Ȳ)
int CClient::SendActiveState()
{
	int retval{ 0 };
	Packet_ActiveState* packet{ nullptr };

	packet = new Packet_ActiveState(Client.GetActiveState());

	clog.Log("INFO", "Send PT_ActiveState");
	std::cout << "Send PT_ActiveState\n";

	retval = Client.ClientSend(packet);
	if (SOCKET_ERROR == retval)
	{
		clog.Log("ERROR", "SendActiveState ClientSend SOCKET_ERROR");
		std::cout << "SendActiveState ClientSend SOCKET_ERROR\n";
	}
	delete packet;

	return retval;
}

//�׽�Ʈ ���ۿ� Ű�Է�(q:ClubSetting, w:TeeSetting, e:active(true), r:active(false))
int CClient::InputKey(const char input)
{
	if ('q' == input)		//Club ���� ����
	{
		return Client.SendClubSetting();
	}
	else if ('w' == input)		//Tee ���� ����
	{
		return Client.SendTeeSetting();
	}
	else if ('e' == input)		//Active ���� (�����->PC �� ���� ���� ����)
	{
		Client.SetActiveState(true);
		return Client.SendActiveState();
	}
	else if ('r' == input)		//Inactive ���� (�����->PC �� �Ұ��� ���� ����)
	{
		Client.SetActiveState(false);
		return Client.SendActiveState();
	}
	else
	{
		return 0;
	}
}

//recv ������
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
		else	//������ �ƴ϶�� ������ �б�
		{
			if (PACKETHEADER == packet.GetSize())
			{
				Client.ReadHeader(packet.GetType());
			}
			else
			{
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
		clog.Log("WARNING", "ReadHeader Recv unknown type");
		std::cout << "ReadHeader Recv unknown type\n";
	}
}

//�߰� ������ recv ��
int CClient::ReadAddData(Packet& packet)
{
	Packet recvpt{};
	int retval{ 0 };

	char* recvdata = (char*)malloc(packet.GetSize());
	if (SOCKET_ERROR == Client.ClientRecv(recvdata, packet.GetSize()))
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

			recvpt.SetType(PACKETTYPE::PT_BallPlaceRecv);
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

			recvpt.SetType(PACKETTYPE::PT_ShotDataRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ActiveState");

			Client.SetActiveState(recvdata);
			std::cout << "Recv PT_ActiveState  // " << Client.GetActiveState() << "\n";

			recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "ReadAddData Recv unknown type");
			std::cout << "ReadAddData Recv unknown type\n";
		}
	}

	//�������� ������ recv �� ���� send ����
	if (PACKETTYPE::PT_None != recvpt.GetType())
	{
		retval = Client.ClientSend(&recvpt);
		if (retval == SOCKET_ERROR)
		{
			clog.Log("ERROR", "ReadAddData ClientSend error");
			std::cout << "ReadAddData ClientSend error\n";
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