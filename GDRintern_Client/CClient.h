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

	//에러 확인 함수
	void err_quit(const char* msg);

	//최초 생성에 따른 데이터, 소켓 설정, 연결 및 스레드 생성
	void DataInit();
	bool ClientInit();
	void ClientConnect();

	//recv 후 패킷 type에 따른 정보 파악
	void ReadData(unsigned int type);

	//pc-pc 통신용 키입력 시 해당 키에 따른 상황 통신
	//w(Tee, club 세팅) e(active 상태 전달) r(inactive 상태 전달), t(샷data 요청 *pc, pc한정)
	int InputKey(const char input);

	//고정데이터 send 후 가변데이터가 존재한다면 가변데이터도 send
	int ClientSend(const void* fixbuf, const void* varbuf, const int varlen);

	//서버로부터 오는 정보 recv
	int ClientRecv(void* buf, int len);

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
	SOCKET m_hSock;
	SOCKADDR_IN m_tAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;

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