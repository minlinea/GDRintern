#include "CServer.h"
#include "conio.h"

//�α�
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

//������ �ʱ�ȭ
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

//��Ű��� �ʱ�ȭ
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

//���� ����
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

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv ������ ����
		Server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.RecvThread, (LPVOID)Server.m_hClient, 0, &dwRecvThreadID);
		Server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.SendThread, (LPVOID)Server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(Server.m_hSend, INFINITE);//Send������ ���� ���(Ŭ���̾�Ʈ���� ���� ���� ���� Ȯ��)
		
		Server.m_bConnect = false;

		TerminateThread(Server.m_hRecv, 0);
		TerminateThread(Server.m_hRecv, 0);

		Server.PrintLog("LOGOUT", "logout : %s", inet_ntoa(tCIntAddr.sin_addr));
	}
	closesocket(Server.m_hClient);
	return;
}

//�α� ��� ����
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

//send ������
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	Server.PrintLog("INFO", "SendThread ON");

	while (true == Server.m_bConnect)
	{
		time(&Server.m_tNowTime);

		Server.InputKey();		//�׽�Ʈ�� Ű�Է� �Լ�

		//��� �ð��� ������ Ŭ���̾�Ʈ�� ���� �Է��� ���� ���
		if (WaitingTime <= Server.m_tNowTime - Server.m_tBeforeTime)	//ConnectCheck ����
		{
			Server.SendPacket<Packet>(PACKETTYPE::PT_ConnectCheck);
			Server.m_tBeforeTime = Server.m_tNowTime;				//BeforeTime ����
			++Server.m_iWaitingCount;								//count ����
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


			if (MAXWaitingCount <= Server.m_iWaitingCount)			//���� count�� �Ѱ�ٸ�
			{
				SuspendThread(Server.m_hSend);						//������ �Ͻ�����
			}
		}
	}
	return NULL;
}

//class P : �����ϰ��� �ϴ� Packet �Ǵ� Packet����Ŭ����
//PACKETDATA : Packet�� ��� PACKETTYPE // Packet����Ŭ������ ��� �����ϰ��� �ϴ� ������
template <class P, class PACKETDATA>
void CServer::SendPacket(PACKETDATA data)
{
	m_qPacket.push(new P(data));
}

//�׽�Ʈ ���ۿ� Ű�Է�(w:ballplace, e:shotdata, r:active(false)
void CServer::InputKey()
{
	if (_kbhit())
	{
		char input = _getch();
		if ('w' == input)		//����ġ ����(enum)
		{
			Server.SendPacket<PacketBallPlace>(Server.GetBallPlace());
		}
		else if ('e' == input)		//������ ����
		{
			Server.SendPacket<PacketShotData>(Server.GetShotData());
		}
		else if ('r' == input)		//�� ���� activestate false ����
		{
			Server.SendPacket<PacketActiveState>(Server.GetActiveState());
		}
	}
}

//recv ������
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
		else    //������ �ƴ϶�� ������ �б�
		{
			Server.PrintLog("INFO", "Recv : %s", to_string(packet.GetType()));

			//���� �Է��� ���� ��� ��� ī��Ʈ �ʱ�ȭ
			Server.m_iWaitingCount = 0;
			ResumeThread(Server.m_hSend);	//SendThread resume
			Server.m_tBeforeTime = Server.m_tNowTime;		//���ð� ����

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

//�߰� ������ recv ��
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