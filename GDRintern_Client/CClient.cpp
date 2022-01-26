#include "CClient.h"
#include "windows.h"
#include "conio.h"

void* CastConversion(PACKETTYPE type)
{
	if (PACKETTYPE::PT_Shot == type)
	{
		auto p = ShotData{};
		return &p;
	}
	else if (PACKETTYPE::PT_Place == type)
	{
		auto p = BALLPLACE{};
		return &p;
	}
	else
	{
		auto p = nullptr_t{};
		return &p;
	}
}


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
	m_eTee = TEE::T40;
	m_eClub = CLUB::DRIVER;

	m_ePlace = BALLPLACE::OB;

	m_bState = false;

	m_iPhase = 0;
	m_fBallSpeed = 0.f;
	m_fLaunchAngle = 0.f;
	m_fLaunchDirection = 0.f;
	m_fHeadSpeed = 0.f;
	m_iBackSpin = 0;
	m_iSideSpin = 0;
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
	if ('w' == input)		//Tee, Club 세팅 변경
	{
		/*Packet pt(PACKETTYPE::PT_Setting, sizeof(Packet));
		TeeClubSetting setting{ m_eTee, m_eClub };
		retval = client.ClientSend(&pt, &setting, sizeof(TeeClubSetting));*/

		std::cout << "Send PT_Setting\n";
	}
	else if ('e' == input)		//Active 상태 (모바일->PC 샷 가능 상태 전달)
	{
		/*Packet pt(PACKETTYPE::PT_Active, sizeof(Packet));
		ACTIVESTATE activestate{ true };
		retval = client.ClientSend(&pt, &activestate, sizeof(ACTIVESTATE));*/

		client.m_hMutex.lock();
		client.m_bState = true;
		client.m_hMutex.unlock();
		std::cout << "Send PT_Active(true)\n";
	}
	else if ('r' == input)		//Inactive 상태 (모바일->PC 샷 불가능 상태 전달)
	{
		//Packet pt(PACKETTYPE::PT_Active, sizeof(Packet));
		//ACTIVESTATE activestate{ false };
		//retval = client.ClientSend(&pt, &activestate, sizeof(ACTIVESTATE));

		client.m_hMutex.lock();
		client.m_bState = false;
		client.m_hMutex.unlock();

		std::cout << "Send PT_Active(false)\n";
	}
	else
	{
	}

	return retval;
}

DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	auto& client = CClient::Instance();
	int retval{ 0 };
	while (true)
	{
		//if (true == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		//{
		//	if (SOCKET_ERROR == client.InputKey(_getch()))
		//	{
		//		std::cout << "SendThread InputKey error\n";
		//		//err_quit("send()");
		//		break;
		//	}//InputKey->Client_Send
		//}
		//else
		//{
		//	//Sleep(2000);		//5초마다 통신(유휴상태 체크)

		//	//Packet pt{ PT_None, sizeof(Packet) };
		//	//if (SOCKET_ERROR == client.ClientSend(&pt, NULL, 0))
		//	//{
		//	//	std::cout << "SendThread Set_Packet error\n";
		//	//	//err_quit("send()");
		//	//	break;
		//	//}//Client_Send
		//}

	}
	return NULL;
}

template <class T>
void CClient::ReadData(Packet<nullptr_t>& header)
{
	auto& client = CClient::Instance();

	T data;
	if (SOCKET_ERROR == client.ClientRecv(&data, header.GetSize()))
	{
		std::cout << "ReadData ClientRecv\n";
	}
	else
	{
		if (PACKETTYPE::PT_Place == header.GetType())
		{
			std::cout << "PT_Place Recv\n";
			//client.SetPlace(static_cast<BALLPLACE>(data));
		}
		else if (PACKETTYPE::PT_ShotDataRecv == header.GetType())
		{
			std::cout << "PT_ShotDataRecv recv\n";
		}
		else if (PACKETTYPE::PT_ShotData == header.GetType())
		{
			std::cout << "PT_ShotData Recv\n";
			//client.SetShotData(static_cast<ShotData>(data));
		}
		else if (PACKETTYPE::PT_ConnectRecv == header.GetType())
		{
			std::cout << "PT_ConnectRecv recv\n";
		}
		else if (PACKETTYPE::PT_Active == header.GetType())
		{
			std::cout << "PT_Active recv\n";
			//client.SetState(static_cast<ACTIVESTATE>(data));
		}
		else
		{
			std::cout << "ReadData unknown type\n";
		}
	}
}

DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	auto& client = CClient::Instance();
	int retval{ 0 };
	Packet<nullptr_t> pt;
	while (true)
	{
		//std::cout << "RecvThread on\n";
		ZeroMemory(&pt, sizeof(pt));
		retval = client.ClientRecv((char*)&pt, sizeof(pt));

		if (SOCKET_ERROR == retval)
		{
			std::cout << "RecvThread ClientRecv error\n";
			//err_quit("recv()");
			break;
		}
		else	//에러가 아니라면 데이터 읽기
		{
			client.ReadData<decltype(CastConversion(pt.GetType()))>(pt);
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

int CClient::ClientSend(const void* buf, const unsigned int size)
{
	auto& server = CClient::Instance();
	return send(server.m_hSock, (const char*)buf, size, 0);
}

int CClient::ClientRecv(void* buf, const int len)
{
	auto& client = CClient::Instance();
	return recv(client.m_hSock, (char*)buf, len, 0);
}

int CClient::TestRecv(void* buf, const int len)
{
	auto& client = CClient::Instance();
	return recv(client.m_hSock, (char*)buf, len, 0);
}

