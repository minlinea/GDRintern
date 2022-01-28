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

enum class BALLPLACE
{
	PAIRWAY,
	TEE,
	OB
};

enum class PACKETTYPE {
	PT_Connect,
	PT_ConnectRecv,
	PT_Active,
	PT_ActiveRecv,
	PT_Place,
	PT_PlaceRecv,
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
		this->size = sizeof(T);
		this->data = nullptr;

		
		this->SetData(data);
	}

	~Packet()
	{
		if (nullptr != this->data)
		{
			free(this->data);
		}
	}
	
	void SetType(const PACKETTYPE& type)
	{
		this->type = type;
	}
	void SetSize(const unsigned int& size)
	{
		this->size = size;
	}

	template <class T>
	void SetData(const T& data)
	{
		if (nullptr != this->data)
		{
			free(this->data);
		}
		
		this->data = (char*)malloc(this->size);
		memcpy_s(this->data, this->size, &data, this->size);
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

	void MakeBuf(char* p)
	{
		p = (char*)malloc(sizeof(Packet) + this->size);
		memcpy_s(p, sizeof(Packet), this, sizeof(Packet));
		if (0 != this->size)
		{
			memcpy_s(p + sizeof(Packet), this->size, this->data, this->size);
		}
		return;
	}

protected:
	PACKETTYPE type;
	unsigned int size;
	char* data;
};

//class PacketShotData : public PacketHeader
//{
//public:
//	PacketShotData()
//	{
//		this->data = ShotData{};
//	}
//	PacketShotData(PACKETTYPE type)
//	{
//		this->type = type;
//		this->size = size;
//		this->data = ShotData{};
//	}
//	PacketShotData(PacketHeader header)
//	{
//		this->type = header.GetType();
//		this->size = header.GetSize();
//		this->data = ShotData{};
//	}
//	PacketShotData(const void *data)
//	{
//		this->data = (ShotData&)data;
//	}
//	~PacketShotData() = default;
//	virtual void PrintLog() override
//	{
//		std::cout << this->size << "\n";
//	}
//private:
//	ShotData data;
//};