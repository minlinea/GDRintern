#include "CClient.h"


int main()
{
	CClient cClient;

	if (true == cClient.ClientInit())
	{
		cClient.ClientConnect();
	}
	
}

