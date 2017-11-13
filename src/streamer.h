#include <stdint.h>
#include <iostream>
#include <cstring>
#include <array>
#include <list>
#include <functional>
#include <chrono>
#include <memory>
#include <thread>
#include "ftd3xx.h"

using namespace std;

class SDR_HEADER
{
protected:
    constexpr SDR_HEADER() {};
    enum class CMD:uint32_t  {TOFIFO = 0, TOCPU = 1};
public:    
    static constexpr bool IsCmd(uint32_t val) { return ((val >> 31) == static_cast<uint32_t>(CMD::TOCPU));}
    static SDR_HEADER* FromRaw(uint32_t val);
};

class F2CPU : protected SDR_HEADER
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
    explicit constexpr F2CPU(uint8_t id, uint8_t num = 0)
    :val(BITS{{.num = num, .id = id, .cmd = static_cast<uint32_t>(CMD::TOCPU) }}){}
    explicit constexpr F2CPU(uint32_t val)
    :val(BITS{.flat = val}) {}

    operator uint32_t() const noexcept {return val.flat;}
    constexpr uint8_t num() const {return val.num;}
    constexpr uint8_t id() const {return val.id;}
};

class F2FIFO : protected SDR_HEADER
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
    constexpr uint16_t num() const {return val.num;}
};    



class OPacketStream
: private streambuf
, public ostream {
public:    
    OPacketStream(FT_HANDLE handle);
    ~OPacketStream() { flush();}

    virtual ostream& flush();

    void SendMessage(uint8_t msgId, const list<uint32_t> &data);

private:
    typedef array<uint32_t, 1023> array_type;
    typedef streambuf::traits_type traits_type;        
    array_type d_buffer;
    const chrono::milliseconds timeout{100};
    FT_HANDLE handle;
    int tx_count;

    void DataReady();
    unsigned int elements() {return (this->pptr() - this->pbase()) / sizeof(uint32_t);}    

    bool SendPacket(const list<uint32_t> &data);

    int overflow(int c);
    int sync();
};


class IPacketStream
: private streambuf
, public istream {
public:
    typedef std::function<void(uint8_t msgId, const list<uint32_t>& body)> Callback_t;

    IPacketStream(FT_HANDLE handle, Callback_t callback);
    ~IPacketStream();

    Callback_t callback;
    thread& GetThread() const {return *read_thread;}

private:
    typedef array<uint32_t, 1024> array_type;
    array_type d_buffer;
    FT_HANDLE handle;
    typedef streambuf::traits_type traits_type;    
    const chrono::milliseconds timeout{1000};
    int rx_count;    
    enum PCKTYPE {NONE, STREAM, MESSAGE} packet_type;
    thread* read_thread;
    char* start;

    void DataReaderThread();
    void DataReaderThreadFile();
    void DataReaderThreadArray();

    void DataReady();

    int overflow(int c);
    int sync();

    
};