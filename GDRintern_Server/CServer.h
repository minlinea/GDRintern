#pragma once
#include <mutex>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib")

#include "packet.h"

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

	//에러 확인 함수
	void err_quit(const char* msg);

	//최초 생성에 따른 데이터, 소켓 설정, 연결 및 스레드 생성
	void DataInit();
	bool ServerInit();
	void ServerAccept();

	//pc-pc 통신용 키입력 시 해당 키에 따른 상황 통신(테스트 코드)
	//w(공위치), e(샷정보), r(ballstate false)
	int InputKey(const char input);
	int SendRecv(const PACKETTYPE& type);

	//recv 후 패킷 type에 따른 정보 파악
	void ReadRecv(const PACKETTYPE& type);
	int ReadData(Packet& type);

	int ServerSend(Packet& packet);
	int ServerRecv(void* buf, const int len);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

	void SetShotData(const ShotData& sd)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_sdShotData = sd;
#ifdef datalog

#endif
	}
	void SetShotData(char* sd)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_sdShotData, sizeof(ShotData), sd, sizeof(ShotData));
#ifdef datalog

#endif
	}
	void SetTeeSetting(const TEESETTING& tee)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eTee = tee;
#ifdef datalog
#endif
	}
	void SetTeeSetting(char* tee)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_sdShotData, sizeof(ShotData), tee, sizeof(ShotData));
#ifdef datalog
#endif
	}
	void SetClubSetting(const CLUBSETTING& club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eClub = club;
#ifdef datalog
#endif
	}
	void SetClubSetting(char* club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_sdShotData, sizeof(ShotData), club, sizeof(ShotData));
#ifdef datalog
#endif
	}
	void SetBallPlace(const BALLPLACE& place)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eBallPlace = place;
#ifdef datalog
		std::cout << "Place:" << (unsigned int)this->m_eBallPlace << "\n";
#endif
	}
	void SetBallPlace(char* ballplace)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_eBallPlace, sizeof(ShotData), ballplace, sizeof(ShotData));
#ifdef datalog
		std::cout << "Place:" << (unsigned int)this->m_eBallPlace << "\n";
#endif
	}
	void SetActiveState(const bool& activestate)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_bActiveState = activestate;
#ifdef datalog
		std::cout << "State:" << this->m_bActiveState << "\n";
#endif
	}
	void SetActiveState(char* activestate)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_bActiveState, sizeof(ShotData), activestate, sizeof(ShotData));
#ifdef datalog
		std::cout << "State:" << this->m_bActiveState << "\n";
#endif
	}

	const TEESETTING GetTeeSetting()
	{
		return this->m_eTee;
	}
	const CLUBSETTING GetClubSetting()
	{
		return this->m_eClub;
	}
	const ShotData GetShotData()
	{
		return m_sdShotData;
	}
	const BALLPLACE GetBallPlace()
	{
		return m_eBallPlace;
	}
	const bool GetActiveState()
	{
		return m_bActiveState;
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
	TEESETTING m_eTee;
	CLUBSETTING m_eClub;

	//POS
	BALLPLACE m_eBallPlace;

	//센서 상태 변경 (active, inactive)
	bool m_bActiveState;

	//ShotData
	ShotData m_sdShotData;
};

