#include "CClient.h"
#include <string>
#include <iostream>
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
	m_eTee = T40;
	m_eClub = DRIVER;

	m_fX = -1;
	m_fY = -1;
	m_fZ = -1;

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
	CClient& client = CClient::Instance();
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

void CClient::InputKey(const char input)
{
	CClient& client = CClient::Instance();
	if ('q' == input)
	{
		
	}
	else if ('w' == input)		//Tee, Club 세팅 변경
	{
		Packet pt(PT_Setting, sizeof(Packet));
		TCSetting setting = { m_eTee, m_eClub };
		client.ClientSend(client.m_hSock, &pt, &setting, 0);

		std::cout << "Send PT_Setting\n";
	}
	else if ('e' == input)		//Active 상태 (모바일->PC 샷 가능 상태 전달)
	{
		Packet pt(PT_Active, sizeof(PT_Active) + sizeof(Packet));
		ACTIVESTATE activestate{ true };
		client.ClientSend(client.m_hSock, &pt, &activestate, sizeof(ACTIVESTATE));

		client.m_bState = true;
		std::cout << "Send PT_Active(true)\n";
	}
	else if ('r' == input)		//Inactive 상태 (모바일->PC 샷 불가능 상태 전달)
	{
		Packet pt(PT_Active, sizeof(PT_Active) + sizeof(Packet));
		ACTIVESTATE activestate{ false };
		client.ClientSend(client.m_hSock, &pt, &activestate, sizeof(ACTIVESTATE));

		client.m_bState = false;
		std::cout << "Send PT_Active(false)\n";
	}
	else if ('t' == input)		//ShotData 요청 (클라이언트 테스트 용, Shot 상황 가정)
	{
		if (true == client.m_bState)
		{
			Packet pt(PT_Shot, sizeof(Packet));
			client.ClientSend(client.m_hSock, &pt, NULL, 0);

			client.m_bState = false;
			std::cout << "Send PT_Shot\n";
		}
		else
		{
			std::cout << "server is not active\n";
		}
	}
	else
	{
	}
}

DWORD WINAPI CClient::SendThread(LPVOID socket)
{
	CClient& client = CClient::Instance();
	std::string sinput;
	int retval = 0;
	while (true)
	{
		//std::lock_guard<std::mutex> lock(client.m_hMutex);
		ZeroMemory(&sinput, sinput.size());
		
		if (true == _kbhit())		//패킷 테스트를 위한 인풋 키 입력
		{
			client.InputKey(_getch());
			//if (SOCKET_ERROR == )
			//{
			//	std::cout << "SendThread Set_Packet error\n";
			//	//err_quit("send()");
			//	break;
			//}//Set_Packet->Client_Send

		}
		else
		{
			//Sleep(2000);		//5초마다 통신(유휴상태 체크)


			//if (SOCKET_ERROR == client.SetPacket((SOCKET)socket, PT_Connect))
			//{
			//	std::cout << "SendThread Set_Packet error\n";
			//	//err_quit("send()");
			//	break;
			//}//Set_Packet->Client_Send
		}

	}

	return NULL;
}

void CClient::ReadData(unsigned int type)
{
	CClient& client = CClient::Instance();

	if (type == PT_Pos)
	{
		POS pos;
		ZeroMemory(&pos, sizeof(pos));
		client.ClientRecv((SOCKET)client.m_hSock, &pos, sizeof(pos));

		client.m_fX = pos.x;
		client.m_fY = pos.y;
		client.m_fZ = pos.z;

		std::cout << client.m_fX << " " << client.m_fY << " " << client.m_fZ << "\n";
	}
	else if (type == PT_ShotData)
	{
		std::cout << "PT_ShotData recv\n";
		ShotData shotdata;
		client.ClientRecv(client.m_hSock, &shotdata, sizeof(ShotData));

		client.m_fBallSpeed = shotdata.ballspeed;
		client.m_fLaunchAngle = shotdata.launchangle;
		client.m_fLaunchDirection = shotdata.launchdirection;
		client.m_fHeadSpeed = shotdata.headspeed;
		client.m_iBackSpin = shotdata.backspin;
		client.m_iSideSpin = shotdata.sidespin;
		std::cout << client.m_fBallSpeed << " " << client.m_fLaunchAngle << " " << client.m_fLaunchDirection
			<< " " << client.m_fHeadSpeed << " " << client.m_iBackSpin << " " << client.m_iSideSpin << "\n";

	}
	else if (type == PT_None)
	{
		std::cout << "PT_None\n";
	}
	else
	{
		std::cout << "recv ok\n";
	}
}

DWORD WINAPI CClient::RecvThread(LPVOID socket)
{
	CClient& client = CClient::Instance();
	int retval = 0;
	while (true)
	{
		//std::cout << "RecvThread Client\n";
		//std::lock_guard<std::mutex> lock(client.m_hMutex);
		//std::cout << "RecvThread\n";
		Packet pt;
		ZeroMemory(&pt, sizeof(pt));
		if (SOCKET_ERROR == client.ClientRecv((SOCKET)client.m_hSock, &pt, sizeof(Packet)))
		{
			std::cout << "Server_Recv error\n";
			//err_quit("recv()");
			break;
		}
		//패킷의 타입을 이용한 함수 추가
		client.ReadData(pt.type);


	}
	return NULL;
}

void CClient::ClientConnect()
{
	CClient& client = CClient::Instance();
	int retval = connect(client.m_hSock, (SOCKADDR*)&client.m_tAddr, sizeof(client.m_tAddr));
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

int CClient::ClientSend(const SOCKET& sock, const void* fixbuf, const void* varbuf, int varlen)
{
	int retval = 0;

	retval = send(sock, (const char*)fixbuf, sizeof(Packet), 0);	//고정데이터 전송
	if (SOCKET_ERROR == retval)
	{
		std::cout << "ClientSend error fixbuf send\n";
	}
	else
	{
		if (0 != varlen)		//가변 데이터에 무언가 있어 추가 전송
		{
			retval = send(sock, (const char*)varbuf, varlen, 0);
			if (SOCKET_ERROR == retval)
			{
				std::cout << "ClientSend error varbuf send\n";
			}
		}
	}
	return retval;
}

int CClient::ClientRecv(const SOCKET& sock, void* buf, int len)
{
	return recv(sock, (char*)buf, len, 0);
}

int CClient::recvn(SOCKET s, char* buf, int len, int flags)

{
	CClient& client = CClient::Instance();
	int received;
	char* ptr = buf;
	int left = len;
	while (left > 0)
	{
		received = recv(client.m_hSock, ptr, left, flags);
		if (received == SOCKET_ERROR)
		{
			return SOCKET_ERROR;
		}
		else if (received == 0)
		{
			break;
		}
		left -= received;
		ptr += received;
	}
	return (len - left);
}
