#include "stream_check_pack.hpp"

/*
========[CommonConfig]=========
*/

CommonConfig::CommonConfig(String tag, int32_t init_value, uint32_t tx_interval, uint32_t check_timeout):
    StreamCheckPack(tx_interval, check_timeout),
    tag(tag),
    target_value(init_value),
    current_drone_value(target_value)
{
    
}


void CommonConfig::rxData(String pack[4], size_t count) {
    if(count < 4) {
        return;
    }
    //Serial.printf("(%u)m_cmd rx: [%s][%s][%s]\n", micros(), pack[1].c_str(), pack[2].c_str(), pack[3].c_str());
    //Serial.printf("m_cmd rx wait value: [%d]\n", wait_ack_value);

    // no ack need (maybe won't be use)
    if(pack[1] == "0") {
        target_value = pack[3].toInt();
    // need ack (drone->wireless)
    }else if(pack[1] == "1") {
        target_value = pack[3].toInt();
        onRxDataFinish();
    // ack pack (drone->wireless)
    }else if(pack[1] == "2") {
        //Serial.printf("m_cmd rx ack valid\n");
        current_drone_value = pack[3].toInt();
        onRxResponseFinish(target_value == current_drone_value);
    }
}


String CommonConfig::getTxDroneStr(uint32_t time) {
    if(isNeedTxData(time)) {
        String tx_str = "comm_cfg,1," + tag + ",";
        tx_str += String(target_value) + "\n";
        onTxDataFinish(time);
        return tx_str;
    }else {
        return "";
    }
}


String CommonConfig::getTxHostStr(uint32_t time) {
    if(isNeedTxResponse(time)) {
        String tx_str = "comm_cfg,2," + tag + ",";
        tx_str += String(current_drone_value) + "\n";
        onTxResponseFinish();
        return tx_str;
    }else {
        return "";
    }
}



/*
========[V4dConfig]=========
*/

V4dConfig::V4dConfig(String tag, vector4_t init_value, uint32_t tx_interval, uint32_t check_timeout):
    StreamCheckPack(tx_interval, check_timeout),
    tag(tag),
    target_value(init_value),
    current_drone_value(init_value)
{

}


void V4dConfig::rxData(String pack[7], size_t count) {
    if(count < 7) {
        return;
    }
    //Serial.printf("(%u)m_cmd rx: [%s][%s][%s]\n", micros(), pack[1].c_str(), pack[2].c_str(), pack[3].c_str());
    //Serial.printf("m_cmd rx wait value: [%d]\n", wait_ack_value);
    vector4_t rx_value = {pack[3].toFloat(), pack[4].toFloat(), pack[5].toFloat(), pack[6].toFloat()};

    // no ack need (maybe won't be use)
    if(pack[1] == "0") {
        //value = pack[3].toInt();
    // need ack (drone->wireless)
    }else if(pack[1] == "1") {
        target_value = rx_value;
        onRxDataFinish();
    // ack pack (drone->wireless)
    }else if(pack[1] == "2") {
        //Serial.printf("m_cmd rx ack valid\n");
        current_drone_value = rx_value;
        onRxResponseFinish(isValueEqual(current_drone_value, target_value));
    }
}


String V4dConfig::getTxDroneStr(uint32_t time) {
    if(isNeedTxData(time)) {
        String tx_str = "v4d_cfg,1," + tag + ",";
        tx_str += String(target_value.w, 5) + ",";
        tx_str += String(target_value.x, 5) + ",";
        tx_str += String(target_value.y, 5) + ",";
        tx_str += String(target_value.z, 5) + "\n";
        onTxDataFinish(time);
        return tx_str;
    }else {
        return "";
    }
}


String V4dConfig::getTxHostStr(uint32_t time) {
    if(isNeedTxResponse(time)) {
        String tx_str = "v4d_cfg,2," + tag + ",";
        tx_str += String(current_drone_value.w, 5) + ",";
        tx_str += String(current_drone_value.x, 5) + ",";
        tx_str += String(current_drone_value.y, 5) + ",";
        tx_str += String(current_drone_value.z, 5) + "\n";
        onTxResponseFinish();
        return tx_str;
    }else {
        return "";
    }
}


bool V4dConfig::isValueEqual(vector4_t& value_a, vector4_t& value_b) {
    const float tolerance = 0.001;
    float diff[4] = {
        abs(value_a.w - value_b.w),
        abs(value_a.x - value_b.x),
        abs(value_a.y - value_b.y),
        abs(value_a.z - value_b.z)
    };
    if(diff[0] < tolerance && diff[1] < tolerance && diff[2] < tolerance && diff[3] < tolerance) {
        return true;
    }else {
        return false;
    }
}

