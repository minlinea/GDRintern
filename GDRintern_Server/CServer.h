#pragma once
#include <mutex>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

#include "../packet.h"

class CServer
{


public:

	CServer();
	~CServer();

	void ServerInit();
	void ServerAccept();

	int Server_Send(const SOCKET& sock, const void* buf, int len);
	int Server_Recv(const SOCKET& sock, void* buf, int len);
	int Set_Packet(const SOCKET& sock, unsigned int type);

	static CServer& Instance()
	{
		static CServer Instance;
		return Instance;
	}

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);
	static DWORD WINAPI ListenThread(LPVOID socket);

private:

	WSADATA m_wsaData;
	SOCKET m_hListen;
	SOCKADDR_IN m_tListenAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;

	SOCKET m_hClient;
};

