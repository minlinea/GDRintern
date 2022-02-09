#pragma once
#pragma pack(1)

#include <iostream>

//////////////////////////////////////////////////////////////////////////////////
// define

#define SERVER_IP "127.0.0.1"
#define PORT 4567
#define PACKETHEADER 8


//////////////////////////////////////////////////////////////////////////////////
// enum

enum class TEESETTING {
	T30,
	T35,
	T40,
	T45,
	T50
};

inline std::ostream& operator << (std::ostream& os, const TEESETTING& t);
inline std::ostream& operator << (std::ostream& os, const TEESETTING& t)
{
	if (TEESETTING::T30 == t)
		std::cout << "T30";
	else if (TEESETTING::T35 == t)
		std::cout << "T35";
	else if (TEESETTING::T40 == t)
		std::cout << "T40";
	else if (TEESETTING::T45 == t)
		std::cout << "T45";
	else if (TEESETTING::T50 == t)
		std::cout << "T50";
	return os;
}
inline const char* to_string(const TEESETTING& t);
inline const char* to_string(const TEESETTING& t)
{
	if (TEESETTING::T30 == t)
		return "T30";
	else if (TEESETTING::T35 == t)
		return "T35";
	else if (TEESETTING::T40 == t)
		return "T40";
	else if (TEESETTING::T45 == t)
		return "T45";
	else if (TEESETTING::T50 == t)
		return "T50";
	return "NONE";
}

enum class CLUBSETTING {
	DRIVER,
	IRON,
	WOOD
};

inline std::ostream& operator << (std::ostream& os, const CLUBSETTING& t);
inline std::ostream& operator << (std::ostream& os, const CLUBSETTING& t)
{
	if (CLUBSETTING::DRIVER == t)
		std::cout << "DRIVER";
	else if (CLUBSETTING::IRON == t)
		std::cout << "IRON";
	else if (CLUBSETTING::WOOD == t)
		std::cout << "WOOD";
	return os;
}
inline const char* to_string(const CLUBSETTING& t);
inline const char* to_string(const CLUBSETTING& t)
{
	if (CLUBSETTING::DRIVER == t)
		return "DRIVER";
	else if (CLUBSETTING::IRON == t)
		return "IRON";
	else if (CLUBSETTING::WOOD == t)
		return "WOOD";
	return "NONE";
}

enum class BALLPLACE
{
	PAIRWAY,
	TEE,
	OB
};

inline std::ostream& operator << (std::ostream& os, const BALLPLACE& t);
inline std::ostream& operator << (std::ostream& os, const BALLPLACE& t)
{
	if (BALLPLACE::PAIRWAY == t)
		std::cout << "PAIRWAY";
	else if (BALLPLACE::TEE == t)
		std::cout << "TEE";
	else if (BALLPLACE::OB == t)
		std::cout << "OB";
	return os;
}

inline const char* to_string(const BALLPLACE& t);
inline const char* to_string(const BALLPLACE& t)
{
	if (BALLPLACE::PAIRWAY == t)
		return "PAIRWAY";
	else if (BALLPLACE::TEE == t)
		return "TEE";
	else if (BALLPLACE::OB == t)
		return "OB";
	return "NONE";
}


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

inline std::ostream& operator << (std::ostream& os, const ShotData& t);
inline std::ostream& operator << (std::ostream& os, const ShotData& t)
{
	std::cout << "phase : " << t.phase << "\nballspeed : " << t.ballspeed
		<< "  launchangle : " << t.launchangle << "  launchdirection : " << t.launchdirection
		<< "\nheadspeed : " << t.headspeed << "  backspin : " << t.backspin << "  sidespin : " << t.sidespin;
	return os;
}

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
	}

	//type이 지정된 생성자(send응답(~~recv 패킷 사용))
	Packet(const PACKETTYPE& type)
	{
		this->type = type;
		this->size = PACKETHEADER;
	}

	virtual ~Packet()
	{

	};

	//type 변경 시
	void SetType(const PACKETTYPE& type)
	{
		this->type = type;
	}

	//size 변경 시(data 전달 시 사용)
	void SetSize(const unsigned int& size)
	{
		this->size = size;
	}

	PACKETTYPE& GetType()
	{
		return this->type;
	}
	unsigned int& GetSize()
	{
		return this->size;
	}

protected:
	PACKETTYPE type;
	unsigned int size;
};


class Packet_ShotData : public Packet
{
public:

	Packet_ShotData()
	{
		this->type = PACKETTYPE::PT_ShotData;
		this->size = PACKETHEADER + sizeof(ShotData);
		this->data = ShotData{};
	}
	Packet_ShotData(const ShotData& data)
	{
		this->type = PACKETTYPE::PT_ShotData;
		this->size = PACKETHEADER + sizeof(ShotData);
		this->data = data;
	}
	~Packet_ShotData() override
	{

	};

	void SetData(const ShotData& data)
	{
		this->data = data;
	}
	ShotData& GetData()
	{
		return this->data;
	}

private:
	ShotData data;
};

class Packet_BallPlace : public Packet
{
public:

	Packet_BallPlace()
	{
		this->type = PACKETTYPE::PT_BallPlace;
		this->size = PACKETHEADER + sizeof(BALLPLACE);
		this->data = BALLPLACE::OB;
	}
	Packet_BallPlace(const BALLPLACE& data)
	{
		this->type = PACKETTYPE::PT_BallPlace;
		this->size = PACKETHEADER + sizeof(BALLPLACE);
		this->data = data;
	}
	~Packet_BallPlace() override
	{

	};

	void SetData(const BALLPLACE& data)
	{
		this->data = data;
	}
	BALLPLACE& GetData()
	{
		return this->data;
	}

private:
	BALLPLACE data;
};

class Packet_ActiveState : public Packet
{
public:

	Packet_ActiveState()
	{
		this->type = PACKETTYPE::PT_ActiveState;
		this->size = PACKETHEADER + sizeof(bool);
		this->data = false;
	}
	Packet_ActiveState(const bool& data)
	{
		this->type = PACKETTYPE::PT_ActiveState;
		this->size = PACKETHEADER + sizeof(bool);
		this->data = data;
	}
	~Packet_ActiveState() override
	{

	};

	void SetData(const bool& data)
	{
		this->data = data;
	}
	bool& GetData()
	{
		return this->data;
	}

private:
	bool data;
};

class Packet_TeeSetting : public Packet
{
public:

	Packet_TeeSetting()
	{
		this->type = PACKETTYPE::PT_TeeSetting;
		this->size = PACKETHEADER + sizeof(TEESETTING);
		this->data = TEESETTING::T40;
	}
	Packet_TeeSetting(const TEESETTING& data)
	{
		this->type = PACKETTYPE::PT_TeeSetting;
		this->size = PACKETHEADER + sizeof(TEESETTING);
		this->data = data;
	}
	~Packet_TeeSetting() override
	{

	};

	void SetData(const TEESETTING& data)
	{
		this->data = data;
	}
	TEESETTING& GetData()
	{
		return this->data;
	}

private:
	TEESETTING data;
};

class Packet_ClubSetting : public Packet
{
public:

	Packet_ClubSetting()
	{
		this->type = PACKETTYPE::PT_TeeSetting;
		this->size = PACKETHEADER + sizeof(Packet_ClubSetting);
		this->data = CLUBSETTING::DRIVER;
	}
	Packet_ClubSetting(const CLUBSETTING& data)
	{
		this->type = PACKETTYPE::PT_TeeSetting;
		this->size = PACKETHEADER + sizeof(TEESETTING);
		this->data = data;
	}
	~Packet_ClubSetting() override
	{

	};

	void SetData(const CLUBSETTING& data)
	{
		this->data = data;
	}
	CLUBSETTING& GetData()
	{
		return this->data;
	}

private:
	CLUBSETTING data;
};