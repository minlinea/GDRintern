#include "CServer.h"


int main()
{
	CServer cServer;
	//cServer.Instance();
	cServer.ServerInit();
	cServer.ServerAccept();
	while (true);
}
