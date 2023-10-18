#ifndef __STREAM_PACK_HPP__
#define __STREAM_PACK_HPP__

#include "core_pack.hpp"


class Ping : public StreamPack {
public:
    Ping(uint32_t send_interval);

    String getTxStr(uint32_t time);
};




class CurrentAttitude : public StreamPack {
public:
    CurrentAttitude(uint32_t send_interval);

    void setData(String* pack);

    String getTxStr(uint32_t time);

private:
    uint32_t dt;
    float attitude[4];
};





class TargetAttitude : public StreamPack {
public:
    TargetAttitude(uint32_t send_interval);

    void setData(String* pack);

    String getTxStr(uint32_t time);

private:
    int16_t attitude[3];
};




class DebugPack : public StreamPack {
public:
    DebugPack(uint32_t send_interval);

    String getTxStr(uint32_t time);
};





#endif