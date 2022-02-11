#include <iostream>
#include <Windows.h>
#include <atlconv.h>
#include <fstream>
#include "conio.h"
#include "windows.h"
#include <ctime>
using namespace std;

enum class PACKETTYPE {
	PT_Connect,
	PT_ConnectRecv,
	PT_ActiveState,
	PT_ActiveStateRecv,
	PT_BallPlace,
	PT_BallPlaceRecv,
	PT_TeeSetting,
	PT_TeeSettingRecv,
	PT_ClubSetting,
	PT_ClubSettingRecv,
	PT_Setting,
	PT_SettingRecv,
	PT_ShotData,
	PT_ShotDataRecv,
	PT_ConnectCheck,
	PT_Disconnect,
	PT_Shot,
	PT_ShotRecv,
	PT_None
};

class aaa
{
public:
	aaa()
	{
		this->size = sizeof(aaa);
		this->type = PACKETTYPE::PT_None;
	}
	aaa(PACKETTYPE type)
	{
		this->size = sizeof(aaa);
		this->type = type;
	}
	virtual ~aaa()
	{

	};

public:
	unsigned int size;
	PACKETTYPE type;

};

class bbb : public aaa
{
public:
	bbb()
	{
		this->size = sizeof(bbb);
		this->type = PACKETTYPE::PT_None;
		this->data = 5;
	}

	bbb(int data)
	{
		this->size = sizeof(bbb);
		this->type = PACKETTYPE::PT_None;
		this->data = data;
	}
public:
	int data;
};

int main()
{
	aaa* a{ nullptr };
	bbb b;
	char c;
	while(true)
	{
		if(1 == _kbhit())	//1이 true인데 true라 하면 경고냄
		{
			c = _getch();

			if (c == 'q')
			{
				cout << sizeof(aaa) << endl;
				cout << sizeof(bbb) << endl;
			}
			else if (c == 'w')
			{
				a = new bbb(7);

				char* t = (char*)malloc(sizeof(bbb));

				memcpy_s(t, sizeof(bbb), a, sizeof(bbb));
				memcpy_s(&b, sizeof(bbb), t, sizeof(bbb));

				aaa k1;
				bbb k2;
				int t1;
				int t2;
				int t3;
				int t4;
				memcpy_s(&t1, 4, a, 4);
				memcpy_s(&t2, 4, a + 4, 4);
				memcpy_s(&t3, 4, a + 8, 4);
				memcpy_s(&t4, 4, a + 12, 4);
				cout << t1 << "\t" << t2 << "\t" << t3 << "\t" << t4 << endl;
				
				memcpy_s(&k1, sizeof(aaa), a, sizeof(aaa));
				cout << k1.size << "\t" << (unsigned int)k1.type << endl;

				memcpy_s(&k2, sizeof(aaa), a, sizeof(aaa));
				cout << k2.size << "\t" << (unsigned int)k2.type << endl;

				cout << b.size << "\t" << b.data << endl;

				free(t);
			}
			
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}