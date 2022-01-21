#pragma once
#include <mutex>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"
class CClient
{
public:

	static CClient& Instance()
	{
		static CClient Instance;
		return Instance;
	}

	CClient();
	~CClient();

	void err_quit(const char* msg);

	void DataInit();
	bool ClientInit();
	void ClientConnect();

	void ReadData(unsigned int type);

	int ClientSend(const SOCKET& sock, const void* buf, int len);
	int ClientRecv(const SOCKET& sock, void* buf, int len);
	int SetPacket(const SOCKET& sock, unsigned int type);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);
	int recvn(SOCKET s, char* buf, int len, int flags);
private:

	WSADATA m_wsaData;
	SOCKET m_hSock;
	SOCKADDR_IN m_tAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;

	//데이터 관련
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