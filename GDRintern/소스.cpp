#include <iostream>
#include "conio.h"
#include "windows.h"
using namespace std;

struct A {
	int pos = 0;
	int len = 0;
};

struct B {
	float x = 0.f;
	float y = 0.f;
	float z = 0.f;
};

A* SetVariableData(unsigned int size, void* data)
{
	A* pt = (A*)malloc(sizeof(A*) + size);

	memcpy_s(pt + sizeof(A), size, data, size);
	cout << "size : " << sizeof(A) + size << endl;
	return pt;
//	free(pt);
}


int main()
{
	char c;
	while(true)
	{
		
		if(true == _kbhit())
		{
			c = _getch();
			cout << c << " " << "test\n";

			if (c == 'q')
			{
				cout << "y\n";
			}
		}
		else
		{
			Sleep(2000);
			cout << "sleep\n";
		}

	}
}