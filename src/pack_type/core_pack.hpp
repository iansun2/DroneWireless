#ifndef __CORE_PACK_HPP__
#define __CORE_PACK_HPP__

#include <Arduino.h>



class BootPack {
private:
    bool need_tx_response;
    bool boot_ok;
    
    uint32_t last_tx_time;

    uint32_t tx_interval;

public:
    const size_t PACK_LENGTH;

    BootPack(uint32_t tx_interval);

    void rxPack(String pack[3]);

    void setBoot(bool stat);

    String getTxDroneStr();

    String getTxHostStr(uint32_t time);

    bool isBoot();
};




/*
==============[Stream]=================
*/

class StreamPack{
private:
    uint32_t last_tx_time;

    uint32_t tx_interval;

public:
    // send to one side and receive from another side
    // if receive new data, transmit it immediately
    // else using send_interval as transmit interval
    /**
     * @brief sream mode
     * @param tx_interval normal send interval
     */
    StreamPack(uint32_t tx_interval);

    bool isNeedTx(uint32_t time);

    void onTxFinish(uint32_t time);

    void onRxFinish();
};




/*
==============[StreamCheck]=================
*/
// pack[1]->    1: send data, need respond current data
//              2: respond current data
// pack[2]->    cmd
// pack[3]->    value

class StreamCheckPack{
private:
    uint32_t tx_interval;
    uint32_t check_timeout;

    uint32_t last_tx_data_time;

    bool need_tx_response;
    bool response_matched;
    
public:
    // send to one side and receive from another side
    // if receive new data, transmit it immediately
    // else using send_interval as transmit interval
    // if response check invalid and reach check_timeout, retransmit immediately
    /**
     * @brief stream + check mode
     * @param tx_interval normal send interval
     * @param check_timeout time for waiting check
     */
    StreamCheckPack(uint32_t tx_interval, uint32_t check_timeout);

    bool isNeedTxData(uint32_t time);
    bool isNeedTxResponse(uint32_t time);

    void onTxDataFinish(uint32_t time);
    void onTxResponseFinish();

    void onRxDataFinish();
    void onRxResponseFinish(bool response_match);
};











#endif