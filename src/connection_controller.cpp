#include "connection_controller.hpp"

/*
========[String->Pack Function]=========
*/

size_t getPackArray(String* pack_array, String& raw_pack, size_t max_count) {
    size_t pack_cnt = 0;
    int start_idx = 0;
    int end_idx = raw_pack.indexOf('\n', start_idx);
    while(end_idx != -1) {
        pack_array[pack_cnt] = raw_pack.substring(start_idx, end_idx);
        start_idx = end_idx + 1;
        end_idx = raw_pack.indexOf('\n', start_idx);
        pack_cnt ++;
        if(pack_cnt >= max_count) {
            break;
        }
    }
    return pack_cnt;
}


size_t getPackSplited(String* pack_splited, String& pack, size_t max_count) {
    size_t data_cnt = 0;
    int start_idx = 0;
    int end_idx = pack.indexOf(',', start_idx);
    while(end_idx != -1) {
        pack_splited[data_cnt] = pack.substring(start_idx, end_idx);
        start_idx = end_idx + 1;
        end_idx = pack.indexOf(',', start_idx);
        data_cnt ++;
        if(data_cnt >= max_count) {
            break;
        }else if(end_idx == -1) {
            pack_splited[data_cnt] = pack.substring(start_idx, pack_splited[data_cnt].length()-1);
            data_cnt ++;
        }
    }
    return data_cnt;
}





/*
========[Private Function]=========
*/

void ConnectionController::configCommonSel(String* cmd_pack) {
    if(cmd_pack[2] == "pwr_on") {
        cfg_comm_pwr_on.rxData(cmd_pack);

    }else if(cmd_pack[2] == "max_pwr") {
        cfg_comm_max_pwr.rxData(cmd_pack);

    }else {
        Serial.printf("undefine command\n");
    }
}



void ConnectionController::bootPack(String* boot_pack) {
    // drone send boot, ack = 1
    if(boot_pack[1] == "1") {
        drone_boot_need_send_ack = true;
        Serial.printf("drone boot ok\n");

    // host send boot ack, ack = 2
    }else if(boot_pack[1] == "2") {
        host_boot_ok = true;
        Serial.printf("host boot ok\n");
    
    }else {
        Serial.printf("boot pack undefine\n");
    }
}



void ConnectionController::rxPackSel(String* splited_pack, size_t count) {
    // target attitude
    if(splited_pack[0] == "t_att" && count == 5) {
        target_attitude.setData(splited_pack);

    // current attitude
    }else if(splited_pack[0] == "c_att" && count == 7) {
        current_attitude.setData(splited_pack);

    // main command
    }else if(splited_pack[0] == "cfg_comm" && count == 4) {
        configCommonSel(splited_pack);

    // k pid
    }else if(splited_pack[0] == "cfg_pid" && count == 5) {

    // ping
    }else if(splited_pack[0] == "ping") {
    
    // undefine normal pack
    }else {
        Serial.printf("undefine normal pack\n");
        // debug print
        Serial.printf("cnt: %u {", count);
        for(size_t data_idx = 0; data_idx < count; data_idx++) {
            Serial.printf("[%s]", splited_pack[data_idx]);
        }
        Serial.printf("}\n");
    }
}



/*
========[Public Function]=========
*/

ConnectionController::ConnectionController():
    // host status
    host_boot_pack("boot,1,0\n"),
    host_boot_ok(false),
    host_boot_last_send_time(0),
    host_boot_retry_interval(500),
    // drone status
    drone_boot_ack_pack("boot,2,0\n"),
    drone_boot_need_send_ack(false),
    drone_timeout(false),
    // object
    ping(1000),
    current_attitude(200),
    target_attitude(200),
    cfg_comm_max_pwr("max_pwr" ,500, 100, 0),
    cfg_comm_pwr_on("pwr_on", 500, 100, 0),
    // timeout
    connecting_hold(true),
    drone_last_rx_time(0),
    host_last_rx_time(0),
    connection_timeout(3000),
    // debug
    debug_pack(1000)
{

}



void ConnectionController::hostRxPack(String& raw_pack) {
    String packs[10];
    size_t pack_cnt = getPackArray(packs, raw_pack, 10);
    //Serial.printf("pack cnt: %u\n", pack_cnt);

    // update last rx time
    if(pack_cnt > 0) {
        host_last_rx_time = millis();
    }

    for(size_t pack_idx = 0; pack_idx < pack_cnt; pack_idx++) {
        //Serial.printf("pack[%u]: [%s]\n", pack_idx, packs[pack_idx].c_str());
        String splited_pack[10];
        size_t data_cnt = getPackSplited(splited_pack, packs[pack_idx], 10);

        // boot pack
        if(splited_pack[0] == "boot" && data_cnt == 3) {
            bootPack(splited_pack);
        
        // normal pack
        }else if(host_boot_ok && data_cnt >= 2) {
            rxPackSel(splited_pack, data_cnt);
        
        // pack invalid or not host not response boot
        }else {
            Serial.printf("host invalid pack\n");
            // debug print
            Serial.printf("data cnt: %u {", data_cnt);
            for(size_t data_idx = 0; data_idx < data_cnt; data_idx++) {
                Serial.printf("[%s]", splited_pack[data_idx]);
            }
            Serial.printf("}\n");
        }
        
    }
}



void ConnectionController::serialRxPack(String& raw_pack) {
    String packs[10];
    size_t pack_cnt = getPackArray(packs, raw_pack, 10);
    //Serial.printf("pack cnt: %u\n", pack_cnt);

    // update last rx time
    if(pack_cnt > 0) {
        drone_last_rx_time = millis();
        drone_timeout = false;
    }

    for(size_t pack_idx = 0; pack_idx < pack_cnt; pack_idx++) {
        //Serial.printf("pack[%u]: [%s]\n", pack_idx, packs[pack_idx].c_str());
        String splited_pack[10];
        size_t data_cnt = getPackSplited(splited_pack, packs[pack_idx], 10);

        // boot pack
        if(splited_pack[0] == "boot" && data_cnt == 3) {
            bootPack(splited_pack);
        
        // normal pack
        }else if(data_cnt >= 2) {
            rxPackSel(splited_pack, data_cnt);
        
        // pack invalid
        }else {
            Serial.printf("drone invalid pack\n");
            // debug print
            /*
            Serial.printf("data cnt: %u {", data_cnt);
            for(size_t data_idx = 0; data_idx < data_cnt; data_idx++) {
                Serial.printf("[%s]", splited_pack[data_idx]);
            }
            Serial.printf("}\n");
            */
        }
        
    }
}



String ConnectionController::hostTxPack() {
    String tx_buffer = "";
    uint32_t time = millis();

    if(host_boot_ok) {
        tx_buffer += cfg_comm_pwr_on.getTxHostStr(time);
        tx_buffer += cfg_comm_max_pwr.getTxHostStr(time);
        tx_buffer += ping.getTxStr(time);
        tx_buffer += current_attitude.getTxStr(time);

    }else if(time - host_boot_last_send_time > host_boot_retry_interval) {
        host_boot_last_send_time = time;
        Serial.printf("send boot to host\n");
        return host_boot_pack;
    }

    return tx_buffer;
}



String ConnectionController::serialTxPack() {
    String tx_buffer = "";
    uint32_t time = millis();

    if(drone_boot_need_send_ack) {
        drone_boot_need_send_ack = false;
        return drone_boot_ack_pack;

    }else {
        tx_buffer += target_attitude.getTxStr(time);
        tx_buffer += cfg_comm_pwr_on.getTxDroneStr(time);
        tx_buffer += cfg_comm_max_pwr.getTxDroneStr(time);
    }

    return tx_buffer;
}



void ConnectionController::connectionWatchdog() {
    uint32_t time = millis();
    //Serial.printf("time: %lu lt: %lu st: %d\n", time, drone_last_rx_time, drone_timeout);
    if(time - drone_last_rx_time > connection_timeout && !drone_timeout) {
        Serial.printf("drone timeout warning\n");
        drone_timeout = true;
    }

    if(time - host_last_rx_time > connection_timeout && host_boot_ok) {
        Serial.printf("host timeout\n");
        host_boot_ok = false;
    }
}

