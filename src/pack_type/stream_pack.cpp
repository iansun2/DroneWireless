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
========[CurrentAttitude]=========
*/

CurrentAttitude::CurrentAttitude(uint32_t send_interval):
    StreamPack(send_interval),
    dt(1000),
    attitude{1,0,0,0}
{

}


void CurrentAttitude::setData(String* splited_pack) {
    dt = splited_pack[2].toInt();
    attitude[0] = splited_pack[3].toFloat();
    attitude[1] = splited_pack[4].toFloat();
    attitude[2] = splited_pack[5].toFloat();
    attitude[3] = splited_pack[6].toFloat();
    //Serial.printf("c_att: %d %d %d %d\n", (int)(attitude[0]*100000), (int)(attitude[1]*100000), (int)(attitude[2]*100000), (int)(attitude[3]*100000));
    onRxFinish();
    //Serial.printf("CA Rx %d\n", dt);
}


String CurrentAttitude::getTxStr(uint32_t time) {
    if(isNeedTx(time)) {
        String send_str = "c_att,0,";
        send_str += String(dt) + ",";
        send_str += String(attitude[0], 5) + ",";
        send_str += String(attitude[1], 5) + ",";
        send_str += String(attitude[2], 5) + ",";
        send_str += String(attitude[3], 5) + "\n";
        onTxFinish(time);
        //Serial.printf("c_att: %s\n", send_str.c_str());
        return send_str;
    }else {
        return "";
    }
}





/*
========[TargetAttitude]=========
*/

TargetAttitude::TargetAttitude(uint32_t send_interval):
    StreamPack(send_interval),
    attitude{0,0,0}
{

}


void TargetAttitude::setData(String* splited_pack) {
    attitude[0] = splited_pack[2].toInt();
    attitude[1] = splited_pack[3].toInt();
    attitude[2] = splited_pack[4].toInt();
    onRxFinish();
}


String TargetAttitude::getTxStr(uint32_t time) {
    if(isNeedTx(time)) {
        String send_str = "t_att,0,";
        send_str += String(attitude[0]) + ",";
        send_str += String(attitude[1]) + ",";
        send_str += String(attitude[2]) + "\n";
        onTxFinish(time);
        return send_str;
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