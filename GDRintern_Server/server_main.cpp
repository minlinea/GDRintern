#include "CServer.h"


int main()
{
	CServer cServer;
	if (true == cServer.ServerInit())		//�������� ���� ���� ��
	{
		cServer.ServerAccept();				//Ŭ���̾�Ʈ ���� ���
	}
}
