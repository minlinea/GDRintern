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

	this->m_eClub = CLUBSETTING::WOOD;

	this->m_eBallPlace = BALLPLACE::OB;

	this->m_bActiveState = false;

	this->m_sdShotData = ShotData{};
}

//��Ű��� �ʱ�ȭ
bool CClient::ClientInit()
{
	if (0 != WSAStartup(MAKEWORD(2, 2), &Client.m_wsaData))
	{
		Client.PrintLog("ERROR", "ClientInit WSAStartup fail");
		return false;
	}

	Client.m_hSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == Client.m_hSock)
	{
		Client.PrintLog("ERROR", "ClientInit ListenSocket fail");
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
		Client.PrintLog("ERROR", "ClientConnect connect error");
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

//�α� ��� ����
void CClient::PrintLog(const char* logtype, const char* logmsg, ...)
{
	va_list args;

	va_start(args, logmsg);

	char msg[MAX_MESSAGE_LEN] = { 0, };
	vsnprintf_s(msg, sizeof(msg), MAX_MESSAGE_LEN, logmsg, args);

	va_end(args);

	std::cout << msg << "\n";

	clog.Log(logtype, msg);
}

//send ������
DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	Client.PrintLog("INFO", "SendThread ON");
	bool connect{ true };

	while (connect)
	{
		Client.InputKey();

		if (true != Client.m_qPacket.empty())
		{
			for (auto p = Client.m_qPacket.front(); true != Client.m_qPacket.empty(); )
			{
				p = Client.m_qPacket.front();
				if (SOCKET_ERROR == Client.ClientSend(p))
				{
					Client.PrintLog("ERROR", "SendThread ClientSend SOCKET_ERROR");
					connect = false;
					break;
				}
				Client.PrintLog("INFO", "Send %s", to_string(p->GetType()));

				delete p;
				Client.m_qPacket.pop();
			}
		}
	}
	return NULL;
}

//class P : �����ϰ��� �ϴ� Packet �Ǵ� Packet����Ŭ����
//PACKETDATA : Packet�� ��� PACKETTYPE // Packet����Ŭ������ ��� �����ϰ��� �ϴ� ������
template <class P, class PACKETDATA>
void CClient::SendPacket(PACKETDATA data)
{
	m_qPacket.push(new P(data));
}

//�׽�Ʈ ���ۿ� Ű�Է�(q:ClubSetting, w:TeeSetting, e:active(true), r:active(false))
void CClient::InputKey()
{
	if (true == _kbhit())
	{
		char input = _getch();
		if ('q' == input)		//Club ���� ����
		{
			Client.SendPacket<PacketClubSetting>(Client.GetClubSetting());
		}
		else if ('w' == input)		//Tee ���� ����
		{
			Client.SendPacket<PacketTeeSetting>(Client.GetTeeSetting());
		}
		else if ('e' == input)		//Active ���� (�����->PC �� ���� ���� ����)
		{
			Client.SetActiveState(true);
			Client.SendPacket<PacketActiveState>(Client.GetActiveState());
		}
		else if ('r' == input)		//Inactive ���� (�����->PC �� �Ұ��� ���� ����)
		{
			Client.SetActiveState(false);
			Client.SendPacket<PacketActiveState>(Client.GetActiveState());
		}
	}
}

//recv ������
DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	Client.PrintLog("INFO", "RecvThread ON");

	Packet packet{};
	while (true)
	{
		ZeroMemory(&packet, sizeof(Packet));

		if (SOCKET_ERROR == Client.ClientRecv(&packet, PACKETHEADER))
		{
			Client.PrintLog("ERROR", "RecvThread ClientRecv error");
			break;
		}
		else	//������ �ƴ϶�� ������ �б�
		{
			Client.PrintLog("INFO", "Recv : %s", to_string(packet.GetType()));

			if (PACKETHEADER != packet.GetSize())
			{
				ResumeThread(Client.m_hSend);
				if (SOCKET_ERROR == Client.ReadAddData(packet))
				{
					Client.PrintLog("ERROR", "RecvThread ReadAddData SOCKET_ERROR");
					break;
				}
			}
		}
	}
	return NULL;
}

//�߰� ������ recv ��
int CClient::ReadAddData(Packet& packet)
{
	unsigned int recvsize{ packet.GetSize() - PACKETHEADER };
	char* recvdata = (char*)malloc(recvsize);
	int retval{ Client.ClientRecv(recvdata, recvsize) };
	
	if (SOCKET_ERROR != retval)
	{
		if (PACKETTYPE::PT_BallPlace == packet.GetType())
		{
			Client.SetBallPlace(recvdata);

			Client.PrintLog("INFO", "BallPalce : %s", to_string(Client.GetBallPlace()));

			Client.SendPacket<Packet>(PACKETTYPE::PT_BallPlaceRecv);
		}
		else if (PACKETTYPE::PT_ShotData == packet.GetType())
		{
			Client.SetShotData(recvdata);

			ShotData sd = Client.GetShotData();
			Client.PrintLog("INFO", "[phase %d] : ballspeed[%f], launchangle[%f], "
				"launchdirection[%f], headspeed[%f], backspin[%d], sidespin[%d]",
				sd.phase, sd.ballspeed, sd.launchangle, sd.launchdirection,
				sd.headspeed, sd.backspin, sd.sidespin);

			Client.SendPacket<Packet>(PACKETTYPE::PT_ShotDataRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			Client.SetActiveState(recvdata);

			Client.PrintLog("INFO", "ActiveState : %s", to_string(Client.GetActiveState()));

			Client.SendPacket<Packet>(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			Client.PrintLog("WARNING", "ReadAddData Recv unknown type");
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