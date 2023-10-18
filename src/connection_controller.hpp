#ifndef __CONNECTION_CONTROLLER_HPP__
#define __CONNECTION_CONTROLLER_HPP__

#include <Arduino.h>
#include "pack_type/stream_pack.hpp"
#include "pack_type/config_pack.hpp"


class ConnectionController{
public:
    ConnectionController();

    void hostRxPack(String& raw_pack);

    void serialRxPack(String& raw_pack);

    String hostTxPack();

    String serialTxPack();

    void connectionWatchdog();

private:
    // host status
    String host_boot_pack;
    bool host_boot_ok;
    uint32_t host_boot_last_send_time;
    uint32_t host_boot_retry_interval;

    // drone status
    String drone_boot_ack_pack;
    bool drone_boot_need_send_ack;
    bool drone_timeout;
    
    // timeout
    bool connecting_hold;
    uint32_t drone_last_rx_time;
    uint32_t host_last_rx_time;
    uint32_t connection_timeout;

    Ping ping;
    CurrentAttitude current_attitude;
    TargetAttitude target_attitude;
    ConfigCommon cfg_comm_pwr_on;
    ConfigCommon cfg_comm_max_pwr;

    DebugPack debug_pack;

    void configCommonSel(String* cmd_pack);

    void rxPackSel(String* splited_pack, size_t count);

    void bootPack(String* boot_pack);
};



#endif