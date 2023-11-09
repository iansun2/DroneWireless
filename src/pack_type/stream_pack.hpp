#ifndef __STREAM_PACK_HPP__
#define __STREAM_PACK_HPP__

#include "core_pack.hpp"



class V4dStream : private StreamPack {
public:
    typedef struct{
        float w, x, y, z;
    }vector4_t;

private:
    String tag;

    vector4_t value;

public:
    V4dStream(String tag, vector4_t init_value, uint32_t tx_interval);

    void setData(vector4_t value);

    void rxData(String pack[7], size_t count);

    String getTxStr(uint32_t time);
};





class Ping : private StreamPack {
public:
    Ping(uint32_t send_interval);

    String getTxStr(uint32_t time);
};





class DebugPack : private StreamPack {
public:
    DebugPack(uint32_t send_interval);

    String getTxStr(uint32_t time);
};





#endif