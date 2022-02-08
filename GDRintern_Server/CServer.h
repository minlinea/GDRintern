#pragma once
#include <mutex>
#include <winsock2.h>
#include "../CLog.h"
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"

//////////////////////////////////////////////////////////////////////////////////
// #define
#define WaitingTime 2
#define MAXWaitingCount 5

//////////////////////////////////////////////////////////////////////////////////
//CServer
/**
* @brief CServer Ŭ����
* @details �̱��� ������ �����Ͽ� �ϳ��� �ν��Ͻ��� �����Ͽ� ��� �ϰ���
*/
class CServer
{

public:

	/*
	* @brief Ŭ���� �ν��Ͻ� ���
	* @return Ŭ���� �ν��Ͻ�
	*/
	static CServer& Instance()
	{
		static CServer Instance;
		return Instance;
	}

	CServer();
	~CServer();

	//���� ������ ���� ������, ���� ����, ���� �� ������ ����
	void DataInit();
	bool ServerInit();
	void ServerAccept();

	//recv �� ��Ŷ type�� ���� ���� �ľ�
	void ReadHeader(const PACKETTYPE& type);

	//recv �� �߰� data recv
	int ReadAddData(Packet& type);

	//pc-pc ��ſ� Ű�Է� �� �ش� Ű�� ���� ��Ȳ ���(�׽�Ʈ �ڵ�)
	//w(����ġ), e(������), r(ballstate false)
	int InputKey(const char input);

	//���� + ���� ������ send
	int ServerSend(Packet& packet);
	
	//�����κ��� ���� ���� recv
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
		memcpy_s(&m_sdShotData, sizeof(TEESETTING), tee, sizeof(TEESETTING));

	}
	void SetClubSetting(const CLUBSETTING& club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		this->m_eClub = club;
	}
	void SetClubSetting(char* club)
	{
		std::lock_guard<std::mutex> _hMutex(this->m_hMutex);
		memcpy_s(&m_sdShotData, sizeof(CLUBSETTING), club, sizeof(CLUBSETTING));
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

	//Tee, Club ������ ����
	TEESETTING m_eTee;
	CLUBSETTING m_eClub;

	//POS
	BALLPLACE m_eBallPlace;

	//���� ���� ���� (active, inactive)
	bool m_bActiveState;

	//ShotData
	ShotData m_sdShotData;

	WSADATA m_wsaData;
	SOCKET m_hListenSock;
	SOCKADDR_IN m_tListenAddr;

	std::mutex m_hMutex;

	//���޻��� ����
	time_t m_tNowTime;
	time_t m_tBeforeTime;
	int m_iWaitingCount;

	HANDLE m_hSend;
	HANDLE m_hRecv;
	HANDLE m_hListen;
	SOCKET m_hClient;
};

