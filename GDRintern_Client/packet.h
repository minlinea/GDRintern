#pragma once
#pragma pack(1)

#include <iostream>

//////////////////////////////////////////////////////////////////////////////////
// define

#define SERVER_IP "127.0.0.1"
#define PORT 4567
#define PACKETHEADER 8
#define datalog 0


//////////////////////////////////////////////////////////////////////////////////
// enum

enum class TEESETTING {
	T30,
	T35,
	T40,
	T45,
	T50
};

enum class CLUBSETTING {
	DRIVER,
	IRON,
	WOOD
};

enum class BALLPLACE
{
	PAIRWAY,
	TEE,
	OB
};

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


//////////////////////////////////////////////////////////////////////////////////
// struct

typedef struct _ShotData {
	int phase;
	float ballspeed;
	float launchangle;
	float launchdirection;
	float headspeed;
	int backspin;
	int sidespin;
}ShotData;

typedef struct _ACTIVESTATE
{
	bool activestate;
}ACTIVESTATE;



/*
Packet
type : PACKETTYPE, 어떤 패킷이 넘어왔는지 구분
size : 해당 타입 패킷의 사이즈
*/

class Packet
{
public:
	//일반 생성자
	Packet()
	{
		this->type = PACKETTYPE::PT_None;
		this->size = PACKETHEADER;
		this->data = nullptr;
	}

	//type이 지정된 생성자(send응답(~~recv 패킷 사용))
	Packet(const PACKETTYPE& type)
	{
		this->type = type;
		this->size = PACKETHEADER;
		this->data = nullptr;
	}

	//type, data 생성자(data 전달 시 사용)
	template <class T>
	Packet(const PACKETTYPE& type, const T& data)
	{
		this->size = sizeof(T) + sizeof(Packet);
		this->data = nullptr;
		this->SetData(this->type, data);
	}

	//	~Packet() = default;
	~Packet()
	{
		if (this->data != nullptr)
		{
			free(this->data);
		}
		this->data = nullptr;
	}

	//type 변경 시
	void SetType(const PACKETTYPE& type)
	{
		this->type = type;
	}

	//size 변경 시(data 전달 시 사용) [패킷 + data크기]
	void SetSize(const unsigned int& size)
	{
		this->size = size + sizeof(Packet);
	}

	//Recv전 size만큼 데이터 공간 확보용
	void SetData()
	{
		this->data = (char*)malloc(this->size);
		memcpy_s(this->data, PACKETHEADER, this, PACKETHEADER);
	}

	//AddData도 전달 시 생성하는 data
	template <class T>
	void SetData(const PACKETTYPE type, const T& data)
	{
		if (this->data != nullptr)
		{
			free(this->data);
		}
		this->data = nullptr;
		this->SetType(type);
		this->SetSize(sizeof(T));

		this->data = (char*)malloc(this->size);
		memcpy_s(this->data, sizeof(Packet), this, sizeof(Packet));
		memcpy_s(this->data + sizeof(Packet), sizeof(T), &data, sizeof(T));
	}

	PACKETTYPE& GetType()
	{
		return this->type;
	}
	unsigned int& GetSize()
	{
		return this->size;
	}
	char* GetData()
	{
		return this->data;
	}

private:
	PACKETTYPE type;
	unsigned int size;
	char* data = nullptr;
};