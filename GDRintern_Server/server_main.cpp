#include "CServer.h"


int main()
{
	CServer cServer;
	//cServer.Instance();
	if (true == cServer.ServerInit())
	{
		if (true == cServer.ServerAccept())
		{
			
			while (true);
		}
	}
}
