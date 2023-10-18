#include "config_pack.hpp"


/*
========[ConfigCommon]=========
*/

ConfigCommon::ConfigCommon(String cmd, uint32_t send_interval, uint32_t ack_timeout, int32_t init_value):
    ConfigPack(send_interval, ack_timeout),
    cmd(cmd),
    value(init_value),
    current_drone_value(value)
{
    
}


void ConfigCommon::rxData(String* pack) {
    //Serial.printf("(%u)m_cmd rx: [%s][%s][%s]\n", micros(), pack[1].c_str(), pack[2].c_str(), pack[3].c_str());
    //Serial.printf("m_cmd rx wait value: [%d]\n", wait_ack_value);

    // no ack need (maybe won't be use)
    if(pack[1] == "0") {
        value = pack[3].toInt();
    // need ack (drone->wireless)
    }else if(pack[1] == "1") {
        value = pack[3].toInt();
        onRxDataFinish();
    // ack pack (drone->wireless)
    }else if(pack[1] == "2" && pack[3].toInt() == value) {
        //Serial.printf("m_cmd rx ack valid\n");
        onRxResponse(value == current_drone_value);
    }
}


String ConfigCommon::getTxDroneStr(uint32_t time) {
    if(isNeedTxData(time)) {
        String send_str = "cfg_comm,1," + cmd + ",";
        send_str += String(value) + "\n";
        onTxDataFinish(time);
        return send_str;
    }else {
        return "";
    }
}


String ConfigCommon::getTxHostStr(uint32_t time) {
    if(isNeedTxResponse(time)) {
        String send_str = "cfg_comm,2," + cmd + ",";
        send_str += String(current_drone_value) + "\n";
        onTxResponseFinish();
        return send_str;
    }else {
        return "";
    }
}