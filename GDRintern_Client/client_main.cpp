#include "CClient.h"


int main()
{
	CClient cClient{};
	if (true == cClient.ClientInit())		//�������� Ŭ���̾�Ʈ ���� ��
	{
		cClient.ClientConnect();			//���� ��� ����
	}
	
}

