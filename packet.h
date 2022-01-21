#pragma once
#pragma pack(1)

#define SERVER_IP "127.0.0.1"
#define PORT 4567

enum TEE {
	T30,
	T35,
	T40,
	T45,
	T50
};

enum CLUB {
	DRIVER,
	IRON,
	WOOD
};


enum PACKETTYPE {
	PT_Connect,
	PT_Active,
	PT_Pos,
	PT_Setting,
	PT_ShotData,
	PT_ConnectCheck,
	PT_Disconnect,
	PT_None
};

;

typedef struct _ShotData {
	int phase;
	float ballspeed;
	float launchangle;
	float launchdirection;
	float headspeed;
	int backspin;
	int sidespin;
}ShotData;

typedef struct _TCsetting {
	TEE tee;
	CLUB club;
}TCsetting;

typedef struct _POS
{
	float x;
	float y;
	float z;
}POS;

typedef struct _ACTIVESTATE
{
	bool state;
}ACTIVESTATE;




typedef struct _Packet
{
	unsigned int type;
	unsigned int size;
	_Packet()
	{
		this->type = PT_None;
		this->size = 8;
	}
	
	_Packet(unsigned int type)
	{
		this->type = type;
		this->size = 8;
	}

	_Packet(unsigned int type, unsigned int size)
	{
		this->type = type;
		this->size = size;
	}

	void SetType(unsigned int type)
	{
		this->type = type;
	}

	void SetSize(unsigned int size)
	{
		this->size = size;
	}


	_Packet* SetVariableData(unsigned int size, void* data)
	{
		_Packet* pt = (_Packet*)malloc(sizeof(_Packet) + size);

		memcpy_s(pt + sizeof(_Packet), size, &data, size);

		return pt;
	}
}Packet;