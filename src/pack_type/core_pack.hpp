#ifndef __CORE_PACK_HPP__
#define __CORE_PACK_HPP__

#include <Arduino.h>

/*
==============[Stream]=================
*/

class StreamPack{
public:
    StreamPack(uint32_t send_interval);

    bool isNeedTx(uint32_t time);

    void onTxFinish(uint32_t time);

    void onRxFinish();

protected:
    uint32_t last_send_time;

    uint32_t send_interval;
};




/*
==============[Config]=================
*/
// pack[1]->    1: send data, need respond current data
//              2: respond current data
// pack[2]->    cmd
// pack[3]->    value

class ConfigPack{
public:
    ConfigPack(uint32_t send_interval, uint32_t ack_timueout);

    bool isNeedTxData(uint32_t time);
    bool isNeedTxResponse(uint32_t time);

    void onTxDataFinish(uint32_t time);
    void onTxResponseFinish();

    void onRxDataFinish();
    void onRxResponse(bool response_match);

private:
    uint32_t tx_interval;
    uint32_t response_match_timeout;

    uint32_t last_send_data_time;

    bool need_tx_response;
    bool response_match;
};











#endif