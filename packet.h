#pragma once
#pragma pack(1)

#define SERVER_IP "127.0.0.1"
#define PORT 4567

#define datalog 0

enum class TEE {
	T30,
	T35,
	T40,
	T45,
	T50
};

enum class CLUB {
	DRIVER,
	IRON,
	WOOD
};


enum class PACKETTYPE {
	PT_Connect,
	PT_ConnectRecv,
	PT_Active,
	PT_Place,
	PT_Setting,
	PT_ShotData,
	PT_ShotDataRecv,
	PT_ConnectCheck,
	PT_Disconnect,
	PT_Shot,
	PT_None
};

enum class BALLPLACE
{
	PAIRWAY,
	TEE,
	OB
};

typedef struct _ShotData {
	int phase;
	float ballspeed;
	float launchangle;
	float launchdirection;
	float headspeed;
	int backspin;
	int sidespin;
}ShotData;

typedef struct _TeeClubSetting {
	TEE tee;
	CLUB club;
}TeeClubSetting;

typedef struct _ACTIVESTATE
{
	bool state;
}ACTIVESTATE;


/*
Packet
type : PACKETTYPE, 어떤 패킷이 넘어왔는지 구분
size : 해당 타입 패킷의 사이즈
*/

template <class T>
class Packet
{
public:
	Packet()
	{
		this->type = PACKETTYPE::PT_None;
		this->size = sizeof(Packet);
	}

	Packet(const PACKETTYPE& type)
	{
		this->type = type;
		this->size = sizeof(Packet);
	}

	Packet(const PACKETTYPE& type, const unsigned int& size, const T& data)
	{
		this->type = type;
		this->size = size;
		this->data = data;
	}

	void SetType(const PACKETTYPE& type)
	{
		this->type = type;
	}
	void SetSize(const unsigned int& size)
	{
		this->size = size;
	}
	void SetData(const T& data)
	{
		this->data = data;
	}

	const PACKETTYPE GetType()
	{
		return this->type;
	}
	const unsigned int GetSize()
	{
		return this->size;
	}
	const T GetData()
	{
		return this->data;
	}

private:
	PACKETTYPE type;
	unsigned int size;
	T data;

};

