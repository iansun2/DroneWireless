#ifndef __MAIN_HPP__
#define __MAIN_HPP__

#include <Arduino.h>

const uint16_t UDP_PORT = 9000;

void droneSerialRxHandler(void* pvParameters);
void droneSerialTxHandler(void* pvParameters);
void wirelessHandler(void* pvParameters);

#endif