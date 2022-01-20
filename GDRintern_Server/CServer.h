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

	bool ServerInit();
	bool ServerAccept();

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
};

