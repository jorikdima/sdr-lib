#include <stdint.h>
#include <iostream>
#include <cstring>
#include <array>
#include <functional>
#include <chrono>

using namespace std;

class SDR_HEADER
{
public:
	enum class CMD:uint32_t  {TOFIFO = 0, TOCPU = 1};

	class F2CPU
	{
		union BITS
		{
			struct 
			{
				uint32_t     : 20;
				uint32_t num : 8;
				uint32_t id  : 3;
				uint32_t cmd : 1;
			};
			uint32_t flat;
		};
		BITS val;
	public:
	    constexpr F2CPU(uint8_t id, uint8_t num = 0)
		:val(BITS{{.num = num, .id = id, .cmd = static_cast<uint32_t>(CMD::TOCPU) }}){}
		constexpr F2CPU(uint32_t val)
		:val(BITS{.flat = val}) {}

		operator uint32_t() const noexcept {return val.flat;}
	};

	class F2FIFO
	{
		union BITS
		{
			struct 
			{
				uint32_t num : 16;
				uint32_t     : 15;
				uint32_t cmd : 1;				
			};
			uint32_t flat;
		};
		BITS val;
	public:
	    explicit constexpr F2FIFO(uint16_t num)
		:val(BITS{{.num = num, .cmd = static_cast<uint32_t>(CMD::TOFIFO)}}){}
		explicit constexpr F2FIFO(uint32_t val)
		:val(BITS{.flat = val}) {}

		operator uint32_t() const {return val.flat;}
	};	
};


class OPacketStream
: private streambuf
, public ostream {
public:	
	OPacketStream();
	~OPacketStream() { flush();}

	virtual ostream& flush();

private:
	typedef array<uint32_t, 3> array_type;
    typedef streambuf::traits_type traits_type;	    
    array_type d_buffer;
    const chrono::milliseconds timeout{100};

    void PacketReady();
    unsigned int elements() {return (this->pptr() - this->pbase()) / sizeof(uint32_t);}	

    int overflow(int c);
    int sync();
};


class IPacketStream
: private streambuf
, public istream {
public:
	IPacketStream():
	streambuf()
	,istream(static_cast<streambuf*>(this))
	{
		this->flags(ios_base::unitbuf);

	
		auto s = sizeof(this->d_buffer);
		auto start = reinterpret_cast<char*>(this->d_buffer.data());
	
		this->setg(start, start, start + s + 1);
}

	~IPacketStream() { flush();}

	virtual ostream& flush();

private:
	typedef array<uint32_t, 3> array_type;
	array_type d_buffer;
    const chrono::milliseconds timeout{100};

	virtual void PacketReady();
};