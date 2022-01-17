#pragma once
//#pragma pack(1)
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
	PT_Disconnect
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
};

typedef struct _POS
{
	float x;
	float y;
	float z;
}POS;

typedef struct _ACTIVESTATE
{
	bool state;
};