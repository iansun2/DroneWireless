#ifndef __CONFIG_PACK_HPP__
#define __CONFIG_PACK_HPP__

#include "core_pack.hpp"


class CommonConfig : private StreamCheckPack {
private:
    String tag;

    int target_value;
    int current_drone_value;

public:
    CommonConfig(String tag, int32_t init_value, uint32_t tx_interval, uint32_t check_timeout);

    void rxData(String pack[4], size_t count);

    String getTxDroneStr(uint32_t time);

    String getTxHostStr(uint32_t time);
};



class V4dConfig : private StreamCheckPack {
private:
    typedef struct{
        float w, x, y, z;
    }vector4_t;

    String tag;

    vector4_t target_value;
    vector4_t current_drone_value;

    bool isValueEqual(vector4_t& value_a, vector4_t& value_b);

public:
    V4dConfig(String tag, vector4_t init_value, uint32_t tx_interval, uint32_t check_timeout);

    void rxData(String pack[7], size_t count);

    String getTxDroneStr(uint32_t time);

    String getTxHostStr(uint32_t time);
};




#endif