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
	closesocket(Instance().m_hListenSock);
	CloseHandle(Instance().m_hSend);
	CloseHandle(Instance().m_hRecv);
	CloseHandle(Instance().m_hListen);
	//CloseHandle(Instance().m_hMutex);
	WSACleanup();
}

//������ �ʱ�ȭ
void CServer::DataInit()
{
	m_eTee = TEESETTING::T40;

	m_eClub = CLUBSETTING::DRIVER;

	m_eBallPlace = BALLPLACE::OB;

	m_bActiveState = false;

	m_sdShotData = ShotData{ 0,1,2,3,4,5,6 };

	time(&m_tNowTime);
	m_tBeforeTime = m_tNowTime;
	m_iWaitingCount = 0;
}

//��Ű��� �ʱ�ȭ
bool CServer::ServerInit()
{
	auto& server = CServer::Instance();
	if (0 != WSAStartup(MAKEWORD(2, 2), &server.m_wsaData))
	{
		clog.Log("ERROR", "ServerInit WSAStartup fail");
		return false;
	}

	server.m_hListenSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == server.m_hListenSock)
	{
		clog.Log("ERROR", "ServerInit ListenSocket fail");
		return false;
	}

	server.m_tListenAddr.sin_family = AF_INET;
	server.m_tListenAddr.sin_port = htons(PORT);
	server.m_tListenAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(server.m_hListenSock, (SOCKADDR*)&server.m_tListenAddr, sizeof(server.m_tListenAddr)))
	{
		clog.Log("ERROR", "ServerInit bind fail");
		return false;
	}

	if (SOCKET_ERROR == listen(server.m_hListenSock, SOMAXCONN))
	{
		clog.Log("ERROR", "ServerInit listen fail");
		return false;
	}

	return true;
}

//���� ����
void CServer::ServerAccept()
{
	auto& server = CServer::Instance();
	while (true)
	{
		SOCKADDR_IN tCIntAddr = {};
		int iCIntSize = sizeof(tCIntAddr);
		server.m_hClient = accept(server.m_hListenSock, (SOCKADDR*)&tCIntAddr, &iCIntSize);

		clog.Log("LOGIN", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[login]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;//accept ���� ��

		DWORD dwSendThreadID, dwRecvThreadID;		//send, recv ������ ����
		server.m_hRecv = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.RecvThread, (LPVOID)server.m_hClient, 0, &dwRecvThreadID);
		server.m_hSend = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)server.SendThread, (LPVOID)server.m_hClient, 0, &dwSendThreadID);

		WaitForSingleObject(server.m_hSend, INFINITE);//Send������ ���� ���(Ŭ���̾�Ʈ���� ���� ���� ���� Ȯ��)
		clog.Log("LOGOUT", inet_ntoa(tCIntAddr.sin_addr));
		std::cout << "[logout]Client IP : " << inet_ntoa(tCIntAddr.sin_addr) << std::endl;
	}
	closesocket(server.m_hClient);
	return;
}

//send ������
DWORD WINAPI CServer::SendThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	clog.Log("INFO", "SendThread ON");
	std::cout << "SendThread ON\n";

	while (true)
	{

		//Packet packet{ PACKETTYPE::PT_ConnectCheck };

		//time(&server.m_tNowTime);
		if (1 == _kbhit())		//��Ŷ �׽�Ʈ�� ���� ��ǲ Ű �Է�
		{
			if (SOCKET_ERROR == server.InputKey(_getch()))
			{
				clog.Log("ERROR", "SendThread InputKey error");
				std::cout << "SendThread InputKey error\n";
				break;
			}
			else
			{
			}
			//server.m_tBeforeTime = server.m_tNowTime;
			//server.m_iWaitingCount = 0;
		}
		else
		{
			//if (WaitingTime <= server.m_tNowTime - server.m_tBeforeTime)
			//{
			//	pt.SetData()
			//	server.ServerSend(packet);

			//	server.m_tBeforeTime = server.m_tNowTime;


			//	clog.Log("INFO", "PT_ConnectCheck");
			//	std::cout << "Send PT_ConnectCheck\n";

			//	++server.m_iWaitingCount;
			//	if (MAXWaitingCount <= server.m_iWaitingCount)
			//	{
			//		SuspendThread(server.m_hSend);
			//	}
			//	else
			//	{
			//	}
			//}
			//else
			//{
			//}
		}


	}
	return NULL;
}

template <class T1, class T2>
void CServer::MakeSendData(Packet*& pt, char*& senddata)
{
	pt = new T1{};
	dynamic_cast<T1*>(pt)->SetData(GetData<T2>());
	senddata = (char*)malloc(pt->GetSize());
	memcpy_s(senddata, pt->GetSize(), pt, pt->GetSize());
}

//�׽�Ʈ ���ۿ� Ű�Է�(w:ballplace, e:shotdata, r:active(false)
int CServer::InputKey(const char input)
{
	auto& server = CServer::Instance();
	Packet* pt{ nullptr };
	char* senddata{ nullptr };
	int retval{ 0 };
	if ('w' == input)		//����ġ ����(enum)
	{
		server.MakeSendData<Packet_ShotData, ShotData>((pt), senddata);
		
		//auto p = server.GetData<Packet_ShotData>();
		//std::cout << p.GetData();
		//clog.Log("INFO", "Send PT_ShotData");
		//std::cout << "Send PT_ShotData\n";
	}
	else if ('e' == input)		//������ ����
	{
		//.SetData(PACKETTYPE::PT_ShotData, server.GetShotData());

		clog.Log("INFO", "Send PT_ShotData");
		std::cout << "Send PT_ShotData\n";
	}
	else if ('r' == input)		//�� ���� activestate false ����
	{
		server.SetActiveState(false);

		//pt.SetData(PACKETTYPE::PT_ActiveState, server.GetActiveState());

		clog.Log("INFO", "Send PT_ActiveState(false)");
		std::cout << "Send PT_ActiveState(false)\n";
	}
	else
	{
		//pt.SetData();
	}

	retval = send(server.m_hClient, (const char*)senddata, pt->GetSize(), 0);
	if (pt != nullptr)
	{
		delete pt;
	}
	return retval;
}

//recv ������
DWORD WINAPI CServer::RecvThread(LPVOID socket)
{
	auto& server = CServer::Instance();

	clog.Log("INFO", "RecvThread ON");
	std::cout << "RecvThread ON\n";

	while (true)
	{
		//Packet packet{};

		//if (SOCKET_ERROR == server.ServerRecv(&packet, sizeof(packet)))
		//{
		//	clog.Log("ERROR", "RecvThread ServerRecv error");
		//	std::cout << "RecvThread ServerRecv error\n";
		//	break;
		//}
		//else    //������ �ƴ϶�� ������ �б�
		//{
		//	server.m_iWaitingCount = 0;
		//	ResumeThread(server.m_hSend);

		//	if (PACKETHEADER == packet.GetSize())
		//	{
		//		server.ReadHeader(packet.GetType());
		//	}
		//	else
		//	{
		//		if (SOCKET_ERROR == server.ReadAddData(packet))
		//		{
		//			clog.Log("ERROR", "ServerRecv ReadAddData error");
		//			std::cout << "ServerRecv ReadAddData error\n";
		//			break;
		//		}
		//	}
		//	server.m_tBeforeTime = server.m_tNowTime;
		//}

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
		clog.Log("WARNING", "Recv ReadHeader unknown type");
		std::cout << "Recv ReadHeader unknown type\n";
	}
}

//�߰� ������ recv ��
int CServer::ReadAddData(Packet& packet)
{
	//auto& server = CServer::Instance();
	//Packet recvpt{};

	////packet.SetData();
	//char* recvdata = (char*)malloc(packet.GetSize());
	//if (SOCKET_ERROR == server.ServerRecv(recvdata, packet.GetSize()))
	//{
	//	clog.Log("ERROR", "ReadAddData SOCKET_ERROR");
	//	std::cout << "ReadAddData SOCKET_ERROR\n";
	//	return SOCKET_ERROR;
	//}
	//else
	//{
	//	if (PACKETTYPE::PT_ClubSetting == packet.GetType())
	//	{
	//		clog.Log("INFO", "Recv PT_ClubSetting");
	//		//server.SetClubSetting(packet.GetData());
	//		//std::cout << "Recv PT_ClubSetting // " << server.GetClubSetting() << "\n";
	//		recvpt.SetType(PACKETTYPE::PT_ClubSettingRecv);
	//	}
	//	else if (PACKETTYPE::PT_TeeSetting == packet.GetType())
	//	{
	//		clog.Log("INFO", "Recv PT_TeeSetting");
	//		//server.SetTeeSetting(packet.GetData());
	//		//std::cout << "Recv PT_TeeSetting // " << server.GetTeeSetting() << "\n";
	//		recvpt.SetType(PACKETTYPE::PT_TeeSettingRecv);
	//	}
	//	else if (PACKETTYPE::PT_ActiveState == packet.GetType())
	//	{
	//		clog.Log("INFO", "Recv PT_BallPlace");
	//		//server.SetActiveState(packet.GetData());
	//		//std::cout << "Recv PT_BallPlace // " << server.GetBallPlace() << "\n";
	//		recvpt.SetType(PACKETTYPE::PT_ActiveStateRecv);
	//	}
	//	else
	//	{
	//		clog.Log("WARNING", "Recv ReadAddData unknown type");
	//		std::cout << "Recv ReadAddData unknown type\n";
	//	}
	//}

	////recvpt.SetData();
	//free(recvdata);

	return 1;
	//return server.ServerSend(recvpt);
}

//send
int CServer::ServerSend(Packet& packet)
{
	auto& server = CServer::Instance();
	//return send(server.m_hClient, (const char*)packet, packet.GetSize(), 0);
	return 1;
}

//recv
int CServer::ServerRecv(void* buf, const int len)
{
	auto& server = CServer::Instance();
	return recv(server.m_hClient, (char*)buf, len, 0);
}


