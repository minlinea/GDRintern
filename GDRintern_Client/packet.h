#pragma once
#pragma pack(1)

#include <iostream>

#define SERVER_IP "127.0.0.1"
#define PORT 4567

#define datalog 0

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
	Packet()
	{
		this->type = PACKETTYPE::PT_None;
		this->size = sizeof(Packet);
		this->data = nullptr;
	}

	Packet(const PACKETTYPE& type)
	{
		this->type = type;
		this->size = sizeof(Packet);
		this->data = nullptr;
	}

	template <class T>
	Packet(const PACKETTYPE& type, const T& data)
	{
		this->type = type;
		this->size = sizeof(T) + sizeof(Packet);
		this->data = nullptr;

		this->SetSendData(data);
	}

	~Packet()
	{
		//if (nullptr != this->data)
		//{
		//	free(this->data);
		//}
	}
	
	void SetType(const PACKETTYPE& type)
	{
		this->type = type;
	}
	void SetSize(const unsigned int& size)
	{
		this->size = size + sizeof(Packet);
	}

	void SetRecvData()
	{
		this->data = (char*)malloc(this->size);
		memcpy_s(this->data, sizeof(Packet), this, sizeof(Packet));
	}

	template <class T>
	void SetSendData(const PACKETTYPE type, const T& data)
	{
		if (nullptr != this->data)
		{
			free(this->data);
		}
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

	void MakeDataMemory()
	{
		this->data = (char*)malloc(this->size);
	}

	void DeleteData()
	{
		free(this->data);
		this->data = nullptr;
	}

private:
	PACKETTYPE type;
	unsigned int size;
	char* data = nullptr;
};