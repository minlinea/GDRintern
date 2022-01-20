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

	bool ClientInit();
	void ClientConnect();

	int Client_Send(const SOCKET& sock, const void* buf, int len);
	int Client_Recv(const SOCKET& sock, void* buf, int len);
	int Set_Packet(const SOCKET& sock, unsigned int type);

	static DWORD WINAPI RecvThread(LPVOID socket);
	static DWORD WINAPI SendThread(LPVOID socket);

private:

	WSADATA m_wsaData;
	SOCKET m_hSock;
	SOCKADDR_IN m_tAddr;

	std::mutex m_hMutex;

	HANDLE m_hSend;
	HANDLE m_hRecv;
};