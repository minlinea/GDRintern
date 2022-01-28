#include <iostream>
#include "conio.h"
#include "windows.h"
#include "../packet.h"
using namespace std;


int main()
{
	char c;
	while(true)
	{
		ShotData sd{0,1,2,3,4,5,6};
		if(1 == _kbhit())	//1이 true인데 true라 하면 경고냄
		{
			c = _getch();
			cout << c << " " << "test\n";

			if (c == 'q')
			{
				Packet pt(PACKETTYPE::PT_ShotData, sd);
				char* buf = (char*)malloc(pt.GetSize() + sizeof(Packet));
				cout << pt.GetSize() + sizeof(Packet) << endl;
				memcpy_s(buf, sizeof(Packet), &pt, sizeof(Packet));

				int size, type;
				memcpy_s(&type, sizeof(int), buf, sizeof(int));
				memcpy_s(&size, sizeof(int), buf + sizeof(int), sizeof(int));

				cout << size << " " << type << endl;
				memcpy_s(buf + sizeof(Packet), sizeof(ShotData), pt.GetData(), sizeof(ShotData));
				//pt.MakeBuf(buf);
				//server.ServerSend(buf, pt.GetSize());


				ShotData t;
				memcpy_s(&t, sizeof(ShotData), buf + sizeof(ShotData), sizeof(ShotData));

				std::cout << "ballspeed:" << sd.ballspeed << "  launchangle:" << sd.launchangle
					<< "  launchdirection:" << sd.launchdirection << "  headspeed:" << sd.headspeed
					<< "  backspin:" << sd.backspin << "  sidespin:" << sd.sidespin << "\n";


				free(buf);
				//std::cout << "PT_ShotData send\n";

			}
		}
		else
		{
			//Sleep(2000);
			//cout << "sleep\n";
		}

	}
}