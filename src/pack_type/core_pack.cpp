#include "core_pack.hpp"


/*
========[StreamPack]=========
*/

StreamPack::StreamPack(uint32_t send_interval):
    last_send_time(0),
    send_interval(send_interval)
{

}


bool StreamPack::isNeedTx(uint32_t time) {
    return time - last_send_time > send_interval;
}


void StreamPack::onTxFinish(uint32_t time) {
    // update last send time
    last_send_time = time;
}


void StreamPack::onRxFinish() {
    // mark need send
    last_send_time = 0;
}






/*
========[ConfigPack]=========
*/

ConfigPack::ConfigPack(uint32_t tx_interval, uint32_t response_match_timeout):
    tx_interval(tx_interval),
    response_match_timeout(response_match_timeout),
    last_send_data_time(0),
    need_tx_response(false),
    response_match(false)
{

}


// =====[isNeedSend]=====

bool ConfigPack::isNeedTxData(uint32_t time) {
    // response not match
    if(!response_match) {
        // using response_match_timeout
        return time - last_send_data_time > response_match_timeout;
    // response match
    }else {
        // using tx_interval
        return time - last_send_data_time > tx_interval;
    }
}

bool ConfigPack::isNeedTxResponse(uint32_t time) {
    return need_tx_response;
}


// =====[onTxFinish]=====

void ConfigPack::onTxDataFinish(uint32_t time) {
    // update timer
    last_send_data_time = time;
}

void ConfigPack::onTxResponseFinish() {
    // mark response send ok
    need_tx_response = false;
}


// =====[onRx]=====

void ConfigPack::onRxDataFinish() {
    // mark need send data
    last_send_data_time = 0;
}


void ConfigPack::onRxResponse(bool response_match) {
    // mark need send response
    need_tx_response = true;
    response_match = response_match;
}