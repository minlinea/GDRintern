#include "CServer.h"


int main()
{
	CServer cServer;
	if (true == cServer.ServerInit())		//정상적인 서버 진행 시
	{
		cServer.ServerAccept();				//클라이언트 접속 대기
	}
}
