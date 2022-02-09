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
	
	time(&m_tNowTime);
	this->m_tBeforeTime = m_tNowTime;
	this->m_iWaitingCount = 0;
}

//��Ű��� �ʱ�ȭ
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

//���� ����
void CServer::ServerAccept()
{
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		Server.m_hClient = accept(Server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);

		clog.Log("LOGIN", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept ���� ��

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv ������ ����
		Server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.RecvThread, (LPVOID)Server.m_hClient, 0, &dwRecvThreadID);
		Server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Server.SendThread, (LPVOID)Server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(Server.m_hSend, INFINITE);//Send������ ���� ���(Ŭ���̾�Ʈ���� ���� ���� ���� Ȯ��)
		clog.Log("LOGOUT", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
	}
	closesocket(Server.m_hClient);
	return;
}

//send ������
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{
		time(&Server.m_tNowTime);
		if (1 == _kbhit())		//��Ŷ �׽�Ʈ�� ���� ��ǲ Ű �Է�
		{
			if (SOCKET_ERROR == Server.InputKey(_getch()))
			{
				break;
			}
			else
			{
				Server.m_tBeforeTime = Server.m_tNowTime;
				Server.m_iWaitingCount = 0;
			}
		}
		else					//��� ��� ��
		{	//��� �ð��� ������ Ŭ���̾�Ʈ�� ���� �Է��� ���� ���
			//if (WaitingTime <= Server.m_tNowTime - Server.m_tBeforeTime)	//ConnectCheck ����
			//{
			//	Packet packet{ PACKETTYPE::PT_ConnectCheck };
			//	if (SOCKET_ERROR == Server.ServerSend(&packet))
			//	{
			//		clog.Log("ERROR", "SendThread ServerSend SOCKET_ERROR");
			//		std::cout << "SendThread ServerSend SOCKET_ERROR\n";
			//		break;
			//	}
			//	else
			//	{
			//		clog.Log("INFO", "PT_ConnectCheck");
			//		std::cout << "Send PT_ConnectCheck\n";

			//		Server.m_tBeforeTime = Server.m_tNowTime;				//BeforeTime ����

			//		++Server.m_iWaitingCount;								//count ����
			//		if (MAXWaitingCount <= Server.m_iWaitingCount)			//���� count�� �Ѱ�ٸ�
			//		{
			//			SuspendThread(Server.m_hSend);						//������ �Ͻ�����
			//		}
			//		else
			//		{
			//		}
			//	}
			//}
			//else
			//{
			//}
		}
	}
	return NULL;
}

//�׽�Ʈ ���ۿ� Ű�Է�(w:ballplace, e:shotdata, r:active(false)
int CServer::InputKey(const char input)
{
	Packet* packet{ nullptr };
	char* senddata{ nullptr };
	int retval{ 0 };

	if ('w' == input)		//����ġ ����(enum)
	{
		packet = new Packet_BallPlace(Server.GetBallPlace());

		clog.Log("INFO", "Send PT_BallPlace");
		std::cout << "Send PT_BallPlace\n";
	}
	else if ('e' == input)		//������ ����
	{
		packet = new Packet_ShotData(Server.GetShotData());

		clog.Log("INFO", "Send PT_ShotData");
		std::cout << "Send PT_ShotData\n";
	}
	else if ('r' == input)		//�� ���� activestate false ����
	{
		Server.SetActiveState(false);

		packet = new Packet_ActiveState(Server.GetActiveState());

		clog.Log("INFO", "Send PT_ActiveState(false)");
		std::cout << "Send PT_ActiveState(false)\n";
	}
	else if ('a' == input)		//�� ���� activestate false ����
	{
		packet = new Packet(PACKETTYPE::PT_ConnectCheck);

		clog.Log("INFO", "Send PT_ConnectCheck");
		std::cout << "Send PT_ConnectCheck\n";
	}
	else
	{
		return retval;
	}

	if (packet != nullptr)					//Ű �Է��� �������� ���(w,e,r)
	{
		retval = Server.TestSend(packet);
		if (SOCKET_ERROR == retval)
		{
			clog.Log("ERROR", "InputKey ServerSend SOCKET_ERROR");
			std::cout << "InputKey ServerSend SOCKET_ERROR\n";
		}
		delete packet;
	}
	return retval;
}

//recv ������
DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	while (true)
	{
		Packet packet{};

		if (SOCKET_ERROR == Server.ServerRecv(&packet, sizeof(packet)))
		{
			clog.Log("ERROR", "RecvThread ServerRecv SOCKET_ERROR");
			std::cout << "RecvThread ServerRecv SOCKET_ERROR\n";
			break;
		}
		else    //������ �ƴ϶�� ������ �б�
		{
			//Server.m_iWaitingCount = 0;
			//ResumeThread(Server.m_hSend);

			//if (sizeof(Packet) == packet.GetSize())
			//{
			//	Server.ReadHeader(packet.GetType());
			//}
			//else
			//{
			//	if (SOCKET_ERROR == Server.ReadAddData(packet))
			//	{
			//		break;
			//	}
			//}
			//Server.m_tBeforeTime = Server.m_tNowTime;
		}
	}
	return NULL;
}

//�߰� ������ ���� header�� �޴� ���
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

//�߰� ������ recv ��
int CServer::ReadAddData(Packet& packet)
{
	Packet recvpt{PACKETTYPE::PT_None};
	int retval{ 0 };

	char* recvdata = (char*)malloc(packet.GetSize());
	if (SOCKET_ERROR == Server.ServerRecv(recvdata, packet.GetSize()))
	{
		clog.Log("ERROR", "ReadAddData ServerRecv SOCKET_ERROR");
		std::cout << "ReadAddData ServerRecv SOCKET_ERROR\n";
		return SOCKET_ERROR;
	}
	else
	{
		if (PACKETTYPE::PT_ClubSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_ClubSetting");
			Server.SetClubSetting(recvdata);
			std::cout << "Recv PT_ClubSetting // " << Server.GetClubSetting() << "\n";
			recvpt.SetType(PACKETTYPE::PT_ClubSettingRecv);
		}
		else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_TeeSetting");
			Server.SetTeeSetting(recvdata);
			std::cout << "Recv PT_TeeSetting // " << Server.GetTeeSetting() << "\n";
			recvpt.SetType(PACKETTYPE::PT_TeeSettingRecv);
		}
		else if (PACKETTYPE::PT_ActiveState == packet.GetType())
		{
			clog.Log("INFO", "Recv PT_BallPlace");
			Server.SetActiveState(recvdata);
			std::cout << "Recv PT_BallPlace // " << Server.GetBallPlace() << "\n";
			recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
		}
		else
		{
			clog.Log("WARNING", "ReadAddData recv unknown type");
			std::cout << "ReadAddData recv unknown type\n";
		}
	}

	//�������� ������ recv �� ���� send ����
	if (PACKETTYPE::PT_None != recvpt.GetType())
	{
		retval = Server.ServerSend(&recvpt);
		if (SOCKET_ERROR == retval)
		{
			clog.Log("ERROR", "ReadAddData ServerSend SOCKET_ERROR");
			std::cout << "ReadAddData ServerSend SOCKET_ERROR\n";
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

//send
int CServer::TestSend(Packet* packet)
{
	int retval{ 0 };
	int sendsize = packet->GetSize();
	char* senddata = nullptr;

	senddata = (char*)malloc(sendsize);
	memcpy_s(senddata, sizeof(PACKETTYPE), &packet->GetType(), sizeof(PACKETTYPE));
	memcpy_s(senddata + sizeof(PACKETTYPE), sizeof(unsigned int), &packet->GetSize(), sizeof(unsigned int));

	//if (PACKETHEADER == packet->GetSize())
	//{
	//	//memcpy_s(senddata, sizeof(T), packet->GetData(), sizeof(T));
	//}

	retval = send(Server.m_hClient, senddata, sendsize, 0);

	free(senddata);
	return retval;
}