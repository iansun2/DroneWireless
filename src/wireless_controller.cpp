#include "wireless_controller.hpp"

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


void print_pack(String* pack, size_t count) {
    Serial.printf("cnt: %u {", count);
    for(size_t data_idx = 0; data_idx < count; data_idx++) {
        Serial.printf("[%s]", pack[data_idx]);
    }
    Serial.printf("}\n");
}



/*
========[Private Function]=========
*/

void WirelessController::rxPackSel(String* pack, size_t count) {
    // v4d stream
    if(pack[0] == "v4d_stream") {
        // target attitude
        if(pack[2] == "t_att") {
            target_attitude.rxData(pack, count);
        // current attitude
        }else if(pack[2] == "c_att") {
            current_attitude.rxData(pack, count);
        }

    // common config
    }else if(pack[0] == "comm_cfg") {
        // power on
        if(pack[2] == "pwr_on") {
            pwr_on_cfg.rxData(pack, count);
        // max power
        }else if(pack[2] == "max_pwr") {
            max_pwr_cfg.rxData(pack, count);
        // rst att
        }else if(pack[2] == "rst_att") {
            rst_att_cfg.rxData(pack, count);
        }

    // v4d config
    }else if(pack[0] == "v4d_cfg") {
        // kpid
        if(pack[2] == "kpid") {
            kpid_cfg.rxData(pack, count);
        }

    // ping
    }else if(pack[0] == "ping") {
    
    // undefine pack
    }else {
        Serial.printf("undefine pack\n");
        print_pack(pack, count);
    }
}



/*
========[Public Function]=========
*/

WirelessController::WirelessController():
    // config
    connection_timeout(3000),
    // host status
    host_boot(500),
    host_last_rx_time(0),
    // drone status
    drone_boot(500),
    drone_last_rx_time(0),
    // object
    ping(1000),
    current_attitude("c_att", {1.0, 0.0, 0.0, 0.0}, 100),
    target_attitude("t_att", {1.0, 0.0, 0.0, 0.0}, 100),
    pwr_on_cfg("pwr_on", 0, 500, 100),
    max_pwr_cfg("max_pwr", 0, 500, 100),
    rst_att_cfg("rst_att", 0, 500, 100),
    kpid_cfg("kpid", {5.0, 0.0, 0.0}, 500, 100)
{   

}



void WirelessController::rxProcess(String& raw_pack, interface_t interface) {
    String packs[10];
    size_t pack_cnt = getPackArray(packs, raw_pack, 10);
    //Serial.printf("pack cnt: %u\n", pack_cnt);

    // update last rx time
    // host
    if(pack_cnt > 0 && interface == Host) {
        // minus 10 because sometimes last_rx_time will faster than current time
        host_last_rx_time = millis()-10;
    // drone
    }else if(pack_cnt > 0 && interface == Drone) {
        // minus 10 because sometimes last_rx_time will faster than current time
        drone_last_rx_time = millis()-10;
    }

    for(size_t pack_idx = 0; pack_idx < pack_cnt; pack_idx++) {
        //Serial.printf("pack[%u]: [%s]\n", pack_idx, packs[pack_idx].c_str());
        String splited_pack[10];
        size_t data_cnt = getPackSplited(splited_pack, packs[pack_idx], 10);

        // boot pack
        if(splited_pack[0] == "boot" && data_cnt == 3) {
            // drone
            if(interface == Drone) {
                Serial.println("rx drone boot pack");
                drone_boot.rxPack(splited_pack);
            // host
            }else if(interface == Host) {
                Serial.println("rx host boot pack");
                host_boot.rxPack(splited_pack);
            // unknown interface
            }else {
                Serial.printf("Unknown interface %d\n", interface);
            }
        // normal pack
        // drone interface or booted host interface
        }else if( interface == Drone || (interface == Host && host_boot.isBoot()) ){
            rxPackSel(splited_pack, data_cnt);
        // Deprecate pack
        }else {
            Serial.printf("Pack Deprecated\n");
            print_pack(splited_pack, data_cnt);
        }
    }
}



String WirelessController::getTxPack(interface_t interface) {
    String tx_buffer = "";
    uint32_t time = millis();

    // host
    if(interface == Host) {
        // boot ok
        if(host_boot.isBoot()) {
            tx_buffer += pwr_on_cfg.getTxHostStr(time);
            tx_buffer += max_pwr_cfg.getTxHostStr(time);
            tx_buffer += rst_att_cfg.getTxHostStr(time);
            tx_buffer += ping.getTxStr(time);
            tx_buffer += current_attitude.getTxStr(time);
            tx_buffer += kpid_cfg.getTxHostStr(time);
        // send boot
        }else {
            tx_buffer += host_boot.getTxHostStr(time);
        }
    // drone
    }else if(interface == Drone) {
        tx_buffer += target_attitude.getTxStr(time);
        tx_buffer += pwr_on_cfg.getTxDroneStr(time);
        tx_buffer += max_pwr_cfg.getTxDroneStr(time);
        tx_buffer += rst_att_cfg.getTxDroneStr(time);
        tx_buffer += kpid_cfg.getTxDroneStr(time);
        tx_buffer += drone_boot.getTxDroneStr();
    }

    return tx_buffer;
}



void WirelessController::connectionWatchdog() {
    uint32_t time = millis();
    //Serial.printf("time: %lu lt: %lu st: %d\n", time, drone_last_rx_time, drone_timeout);
    // drone
    if(time - drone_last_rx_time > connection_timeout && drone_boot.isBoot()) {
        Serial.printf("drone timeout warning! curr:%lu last:%lu\n", time, host_last_rx_time);
        drone_boot.setBoot(false);
    }
    // host
    if(time - host_last_rx_time > connection_timeout && host_boot.isBoot()) {
        Serial.printf("host timeout! curr:%lu last:%lu\n", time, host_last_rx_time);
        host_boot.setBoot(false);
    }
}

