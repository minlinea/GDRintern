#include "CClient.h"


int main()
{
	CClient cClient{};
	if (true == cClient.ClientInit())		//정상적인 클라이언트 구동 시
	{
		cClient.ClientConnect();			//서버 통신 진행
	}
	
}

