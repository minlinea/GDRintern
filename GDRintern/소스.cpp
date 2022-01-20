#include <iostream>
using namespace std;

struct A {
	int pos = 0;
	int len = 0;
};

int main()
{



	struct A* p = (A*)malloc(sizeof(A) + sizeof(float) * 3);
	p->len = 0;
	p->pos = 0;
	float x[3] = { 3,4,5 };
	for (int i = 0; i < 3; ++i)
	{
		memcpy_s(p + sizeof(A) + sizeof(float) * i, sizeof(float), &x[i], sizeof(float));
		
	}
	for (int i = 0; i < 3; ++i)
	{
		float k = 0, f;
		memcpy_s(&k, sizeof(float),(p + sizeof(A) + sizeof(float) * i), sizeof(float));
		cout << k << endl;
	}
	cout << sizeof(p) << endl;
	free(p);
}