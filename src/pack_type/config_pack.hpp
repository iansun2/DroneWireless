#ifndef __CONFIG_PACK_HPP__
#define __CONFIG_PACK_HPP__

#include "core_pack.hpp"


class ConfigCommon : protected ConfigPack {
public:
    ConfigCommon(String cmd, uint32_t send_interval, uint32_t ack_timeout, int32_t init_value);

    void rxData(String* pack);

    String getTxDroneStr(uint32_t time);

    String getTxHostStr(uint32_t time);

private:
    String cmd;

    int value;
    int current_drone_value;
};








#endif