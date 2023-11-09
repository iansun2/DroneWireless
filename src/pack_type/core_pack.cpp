#include "core_pack.hpp"

/*
========[BootPack]=========
*/

BootPack::BootPack(uint32_t tx_interval):
    PACK_LENGTH(3),
    need_tx_response(false),
    boot_ok(false),
    last_tx_time(0),
    tx_interval(tx_interval)
{

}

void BootPack::rxPack(String pack[3]) {
    if(pack[1] == "1") {
        need_tx_response = true;
    }else if(pack[1] == "2") {
        boot_ok = true;
    }
}

void BootPack::setBoot(bool stat) {
    boot_ok = stat;
}

String BootPack::getTxDroneStr() {
    if(need_tx_response) {
        need_tx_response = false;
        boot_ok = true;
        return "boot,2,0\n";
    }else {
        return "";
    }
}

String BootPack::getTxHostStr(uint32_t time) {
    if(!boot_ok && time - last_tx_time > tx_interval) {
        return "boot,1,0\n";
    }else {
        return "";
    }
}

bool BootPack::isBoot() {
    return boot_ok;
}



/*
========[StreamPack]=========
*/

StreamPack::StreamPack(uint32_t tx_interval):
    last_tx_time(0),
    tx_interval(tx_interval)
{

}

bool StreamPack::isNeedTx(uint32_t time) {
    return time - last_tx_time > tx_interval;
}

void StreamPack::onTxFinish(uint32_t time) {
    // update last tx time
    last_tx_time = time;
}

void StreamPack::onRxFinish() {
    // make tx immediately
    last_tx_time = 0;
}



/*
========[StreamCheckPack]=========
*/

StreamCheckPack::StreamCheckPack(uint32_t tx_interval, uint32_t check_timeout):
    tx_interval(tx_interval),
    check_timeout(check_timeout),
    last_tx_data_time(0),
    need_tx_response(false),
    response_matched(false)
{

}


// [isNeedTx]

bool StreamCheckPack::isNeedTxData(uint32_t time) {
    // response not match
    if(!response_matched) {
        // using check_timeout
        return time - last_tx_data_time > check_timeout;
    // response match
    }else {
        // using tx_interval
        return time - last_tx_data_time > tx_interval;
    }
}

bool StreamCheckPack::isNeedTxResponse(uint32_t time) {
    return need_tx_response;
}


// [onTxFinish]

void StreamCheckPack::onTxDataFinish(uint32_t time) {
    // update timer
    last_tx_data_time = time;
    response_matched = false;
}

void StreamCheckPack::onTxResponseFinish() {
    // mark response send ok
    need_tx_response = false;
}


// [onRx]

void StreamCheckPack::onRxDataFinish() {
    // mark need send data
    last_tx_data_time = 0;
}


void StreamCheckPack::onRxResponseFinish(bool response_match) {
    // mark need send response
    need_tx_response = true;
    this->response_matched = response_match;
}