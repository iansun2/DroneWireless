#include "stream_pack.hpp"

/*
========[PingPack]=========
*/

Ping::Ping(uint32_t send_interval):
    StreamPack(send_interval)
{

}


String Ping::getTxStr(uint32_t time) {
    if(isNeedTx(time)) {
        String send_str = "ping,0,";
        send_str += String(time) + "\n";
        onTxFinish(time);
        return send_str;
    }else {
        return "";
    }
}




/*
========[V4dStream]=========
*/

V4dStream::V4dStream(String tag, vector4_t init_value, uint32_t tx_interval):
    tag(tag),
    value(init_value),
    StreamPack(tx_interval)
{

}


void V4dStream::setData(vector4_t value) {
    this->value = value;
}


void V4dStream::rxData(String pack[7], size_t count) {
    if(count < 7) {
        return;
    }
    value.w = pack[3].toFloat();
    value.x = pack[4].toFloat();
    value.y = pack[5].toFloat();
    value.z = pack[6].toFloat();
    onRxFinish();
    //Serial.printf("CA Rx %d\n", dt);
}


String V4dStream::getTxStr(uint32_t time) {
    if(isNeedTx(time)) {
        String tx_str = "v4d_stream,0," + tag + ",";
        tx_str += String(value.w, 5) + ",";
        tx_str += String(value.x, 5) + ",";
        tx_str += String(value.y, 5) + ",";
        tx_str += String(value.z, 5) + "\n";
        onTxFinish(time);
        //Serial.printf("c_att: %s\n", send_str.c_str());
        return tx_str;
    }else {
        return "";
    }
}





/*
========[DebugPack]=========
*/

DebugPack::DebugPack(uint32_t send_interval):
    StreamPack(send_interval)
{

}


String DebugPack::getTxStr(uint32_t time) {
    static uint8_t cnt = 0;
    String send_str = "debug,0,";
    send_str += String(time) + ",";
    send_str += String(cnt) + "\n";
    cnt ++;
    onTxFinish(time);
    return send_str;
}