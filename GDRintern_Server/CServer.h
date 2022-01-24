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
	void ServerAccept();
	void ConnectInit();	//클라이언트 최초 연결 후 초기화 관련 Send
	void ReadData(unsigned int type);

	int ServerSend(const SOCKET& sock, const void* fixbuf, const void* varbuf, int varlen);
	int ServerRecv(const SOCKET& sock, void* buf, int len);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

	void SetShotData(ShotData& sd)
	{
		this->m_iPhase = sd.phase;
		this->m_fBallSpeed = sd.ballspeed;
		this->m_fLaunchAngle = sd.launchangle;
		this->m_fLaunchDirection = sd.launchdirection;
		this->m_fHeadSpeed = sd.headspeed;
		this->m_iBackSpin = sd.backspin;
		this->m_iSideSpin = sd.sidespin;
	}
	void SetTCSetting(TCSetting& tcs)
	{
		this->m_eTee = tcs.tee;
		this->m_eClub = tcs.club;
	}
	void SetPos(POS& pos)
	{
		this->m_fX = pos.x;
		this->m_fY = pos.y;
		this->m_fZ = pos.z;
	}

	const TCSetting GetTCSetting()
	{
		return TCSetting{ this->m_eTee, this->m_eClub };
	}
	const ShotData GetShotData()
	{
		return ShotData{ this->m_iPhase, this->m_fBallSpeed, this->m_fLaunchAngle,
			this->m_fLaunchDirection, this->m_fHeadSpeed, this->m_iBackSpin, this->m_iSideSpin };
	}
	const POS GetPos()
	{
		return POS{ this->m_fX, this->m_fY, this->m_fZ };
	}

private:

	WSADATA m_wsaData;
	SOCKET m_hListenSock;
	SOCKADDR_IN m_tListenAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;
	HANDLE m_hListen;
	SOCKET m_hClient;
	

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

