#pragma once
#include <mutex>
#include <winsock2.h>
#include <iostream>
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"

#define datalog 0

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

	//���� Ȯ�� �Լ�
	void err_quit(const char* msg);

	//���� ������ ���� ������, ���� ����, ���� �� ������ ����
	void DataInit();
	bool ServerInit();
	void ServerAccept();

	//pc-pc ��ſ� Ű�Է� �� �ش� Ű�� ���� ��Ȳ ���(�׽�Ʈ �ڵ�)
	//w(Shotdata ����)
	int InputKey(const char input);

	//recv �� ��Ŷ type�� ���� ���� �ľ�
	void ReadData(PACKETTYPE type);

	int ServerSend(const void* buf, const unsigned int size);
	int ServerRecv(void* buf, const int len);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

	void SetShotData(const ShotData& sd)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_iPhase = sd.phase;
		this->m_fBallSpeed = sd.ballspeed;
		this->m_fLaunchAngle = sd.launchangle;
		this->m_fLaunchDirection = sd.launchdirection;
		this->m_fHeadSpeed = sd.headspeed;
		this->m_iBackSpin = sd.backspin;
		this->m_iSideSpin = sd.sidespin;
#ifdef datalog
		std::cout << "ballspeed:" << this->m_fBallSpeed << "  launchangle:" << this->m_fLaunchAngle
			<< "  launchdirection:" << this->m_fLaunchDirection << "  headspeed:" << this->m_fHeadSpeed
			<< "  backspin:" << this->m_iBackSpin << "  sidespin:" << this->m_iSideSpin << "\n";
#endif
	}
	void SetTeeClubSetting(const TeeClubSetting& tcs)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eTee = tcs.tee;
		this->m_eClub = tcs.club;
#ifdef datalog
		std::cout << "Tee:" << (unsigned int)this->m_eTee << "  Club:" << (unsigned int)this->m_eClub << "\n";
#endif
	}
	void SetPlace(const BALLPLACE& place)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_ePlace = place;
#ifdef datalog
		std::cout << "Place:" << (unsigned int)this->m_ePlace << "\n";
#endif
	}
	void SetState(const bool& state)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_bState = state;
#ifdef datalog
		std::cout << "State:" << this->m_bState << "\n";
#endif
	}

	const TeeClubSetting GetTeeClubSetting()
	{
		return TeeClubSetting{ this->m_eTee, this->m_eClub };
	}
	const ShotData GetShotData()
	{
		return ShotData{ this->m_iPhase, this->m_fBallSpeed, this->m_fLaunchAngle,
			this->m_fLaunchDirection, this->m_fHeadSpeed, this->m_iBackSpin, this->m_iSideSpin };
	}
	const BALLPLACE GetPlace()
	{
		return m_ePlace;
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
	
	//Tee, Club ������ ����
	TEE m_eTee;
	CLUB m_eClub;

	//POS
	BALLPLACE m_ePlace;

	//���� ���� ���� (active, inactive)
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

