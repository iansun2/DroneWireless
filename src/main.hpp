#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <Arduino.h>
#include "buffer_helper.hpp"


const uint16_t SERVER_PORT = 9000;
const size_t BUF_0_COUNT = 200;
const size_t BUF_1_COUNT = 200;


typedef struct{
    uint16_t flag;
    uint8_t type;
    uint8_t size;
}header_t;


typedef struct{
    uint16_t frame;
    uint16_t dt;
    float qw;
    float qx;
    float qy;
    float qz;
}imu_data_t;


typedef struct{
    header_t header;
    int32_t pitch;
    int32_t roll;
    int32_t yaw;
}target_attitude_t;




void droneSerialRxHandler(void* pvParameters);
void droneSerialTxHandler(void* pvParameters);
void wirelessTxHandler(void* pvParameters);
void wirelessRxHandler(void* pvParameters);
void wirelessConnectionHandler();

void clearBuffer(uint8_t buf_id);
BufferHelper* getWriteableBufferHelper();
BufferHelper* getReadableBufferHelper();

#endif