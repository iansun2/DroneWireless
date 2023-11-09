#ifndef __CONNECTION_CONTROLLER_HPP__
#define __CONNECTION_CONTROLLER_HPP__

#include <Arduino.h>
#include "pack_type/stream_pack.hpp"
#include "pack_type/stream_check_pack.hpp"
#include "pack_type/core_pack.hpp"


class WirelessController{
private:
    // config
    uint32_t connection_timeout;

    // host status
    BootPack host_boot;
    uint32_t host_last_rx_time;

    // drone status
    BootPack drone_boot;
    uint32_t drone_last_rx_time;

    // pack object
    Ping ping;
    V4dStream current_attitude;
    V4dStream target_attitude;
    CommonConfig pwr_on_cfg;
    CommonConfig max_pwr_cfg;
    CommonConfig rst_att_cfg;
    V4dConfig kpid_cfg;

    void rxPackSel(String* splited_pack, size_t count);

    
public:
    typedef enum{
        Drone,
        Host
    }interface_t;

    WirelessController();

    void rxProcess(String& raw_pack, interface_t interface);

    String getTxPack(interface_t interface);

    void connectionWatchdog();
};



#endif