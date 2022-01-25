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

	//에러 확인 함수
	void err_quit(const char* msg);

	//최초 생성에 따른 데이터, 소켓 설정, 연결 및 스레드 생성
	void DataInit();
	bool ServerInit();
	void ServerAccept();

	//recv 후 패킷 type에 따른 정보 파악
	void ReadData(PACKETTYPE type);

	int ServerSend(const void* fixbuf, const void* varbuf, const int varlen);
	int ServerRecv(void* buf, const int len);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

	void SetShotData(const ShotData& sd)
	{
		this->m_iPhase = sd.phase;
		this->m_fBallSpeed = sd.ballspeed;
		this->m_fLaunchAngle = sd.launchangle;
		this->m_fLaunchDirection = sd.launchdirection;
		this->m_fHeadSpeed = sd.headspeed;
		this->m_iBackSpin = sd.backspin;
		this->m_iSideSpin = sd.sidespin;
	}
	void SetTCSetting(const TCSetting& tcs)
	{
		this->m_eTee = tcs.tee;
		this->m_eClub = tcs.club;
	}
	void SetPos(const POS& pos)
	{
		this->m_fX = pos.x;
		this->m_fY = pos.y;
		this->m_fZ = pos.z;
	}
	void SetState(const bool& state)
	{
		this->m_bState = state;
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
	const bool GetState()
	{
		return m_bState;
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
	
	//Tee, Club 데이터 관련
	TEE m_eTee;
	CLUB m_eClub;

	//POS
	float m_fX;
	float m_fY;
	float m_fZ;

	//센서 상태 변경 (active, inactive)
	bool m_bState;

	//ShotData
	int m_iPhase;
	float m_fBallSpeed;
	float m_fLaunchAngle;
	float m_fLaunchDirection;
	float m_fHeadSpeed;
	int m_iBackSpin;
	int m_iSideSpin;
};

