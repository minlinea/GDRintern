#pragma once
#include <mutex>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"

class CServer
{


public:

	static CServer& Instance()
	{
		static CServer Instance;
		return Instance;
	}

	CServer();
	~CServer();

	void err_quit(const char* msg);

	void DataInit();
	bool ServerInit();
	bool ServerAccept();
	void ConnectInit();	//Ŭ���̾�Ʈ ���� ���� �� �ʱ�ȭ ���� Send


	int Server_Send(const SOCKET& sock, const void* buf, int len);
	int Server_Recv(const SOCKET& sock, void* buf, int len);
	int Set_Packet(const SOCKET& sock, unsigned int type);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);
	static DWORD WINAPI ListenThread(LPVOID socket);

private:

	WSADATA m_wsaData;
	SOCKET m_hListenSock;
	SOCKADDR_IN m_tListenAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;
	HANDLE m_hListen;
	SOCKET m_hClient;
	

	//������ ����
	TEE m_eTee;
	CLUB m_eClub;

	float m_fX;
	float m_fY;
	float m_fZ;

	bool m_bState;

	int m_iPhase;
	float m_fBallSpeed;
	float m_fLaunchAngle;
	float m_fLaunchDirection;
	float m_fHeadSpeed;
	int m_iBackSpin;
	int m_iSideSpin;
};

