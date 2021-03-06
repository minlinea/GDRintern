#pragma once
#include <mutex>
#include <queue>
#include "../CLog.h"
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"

//////////////////////////////////////////////////////////////////////////////////
// #define
#define WaitingTime 5
#define MAXWaitingCount 10

//////////////////////////////////////////////////////////////////////////////////
//CServer
/**
* @brief CServer 클래스
* @details 싱글톤 패턴을 적용하여 하나의 인스턴스만 생성하여 사용 하게함
*/
class CServer
{

public:

	/*
	* @brief 클래스 인스턴스 얻기
	* @return 클래스 인스턴스
	*/
	static CServer& Instance()
	{
		static CServer Instance;
		return Instance;
	}

	CServer();
	~CServer();

	//최초 생성에 따른 데이터, 소켓 설정, 연결 및 스레드 생성
	void DataInit();
	bool ServerInit();
	void ServerAccept();

	//recv 후 추가 data recv
	int ReadAddData(Packet& type);

	//pc-pc 통신용 키입력 시 해당 키에 따른 상황 통신(테스트 코드)
	//w(공위치), e(샷정보), r(ballstate false)
	void InputKey();

	//데이터 전송 관련
	//class P : 전송하고자 하는 Packet 또는 Packet하위클래스
	//PACKETDATA : Packet인 경우 PACKETTYPE // Packet하위클래스인 경우 전송하고자 하는 데이터
	template <class P, class PACKETDATA>
	void SendPacket(PACKETDATA data);

	//packet*가 들어오면 안에서 조립해서 보내보기
	int ServerSend(Packet* packet);
	
	//서버로부터 오는 정보 recv
	int ServerRecv(void* buf, const int len);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

	void SetShotData(const ShotData& sd)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_sdShotData = sd;
	}
	void SetShotData(char* sd)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_sdShotData, sizeof(ShotData), sd, sizeof(ShotData));
	}
	void SetTeeSetting(const TEESETTING& tee)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eTee = tee;
	}
	void SetTeeSetting(char* tee)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_eTee, sizeof(TEESETTING), tee, sizeof(TEESETTING));

	}
	void SetClubSetting(const CLUBSETTING& club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eClub = club;
	}
	void SetClubSetting(char* club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_eClub, sizeof(CLUBSETTING), club, sizeof(CLUBSETTING));
	}
	void SetBallPlace(const BALLPLACE& place)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eBallPlace = place;
	}
	void SetBallPlace(char* ballplace)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_eBallPlace, sizeof(BALLPLACE), ballplace, sizeof(BALLPLACE));
	}
	void SetActiveState(const bool& activestate)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_bActiveState = activestate;
	}
	void SetActiveState(char* activestate)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_bActiveState, sizeof(bool), activestate, sizeof(bool));
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
		return this->m_sdShotData;
	}
	const BALLPLACE GetBallPlace()
	{
		return this->m_eBallPlace;
	}
	const bool GetActiveState()
	{
		return this->m_bActiveState;
	}

	//로그 출력 관련
	void PrintLog(const char* logtype, const char* logmsg, ...);

private:

	
	//Tee, Club 데이터 관련
	TEESETTING m_eTee;
	CLUBSETTING m_eClub;

	//POS
	BALLPLACE m_eBallPlace;

	//센서 상태 변경 (active, inactive)
	bool m_bActiveState;

	//ShotData
	ShotData m_sdShotData;

	//유휴상태 관련
	time_t m_tNowTime;
	time_t m_tBeforeTime;
	int m_iWaitingCount;

	//통신관련
	WSADATA m_wsaData;
	SOCKET m_hListenSock;
	SOCKADDR_IN m_tListenAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;
	HANDLE m_hListen;
	SOCKET m_hClient;

	bool m_bConnect;

	std::queue<Packet*> m_qPacket;
};


// CServer 클래스 인스턴스 매크로
#define Server CServer::Instance()