#include <iostream>
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
	

	B t = { 3,4,5 };
	struct A* p = SetVariableData(sizeof(float) * 3, &t);

	p->len = 0;
	p->pos = 0;

	memcpy_s(p+sizeof(A), sizeof(B),&t, sizeof(B));


	B k = {};
	memcpy_s(&k, sizeof(B), p + sizeof(A), sizeof(B));
	
	cout << k.x << endl;
	cout << k.y << endl;
	cout << k.z << endl;


	cout << sizeof(p) << endl;
	free(p);
}