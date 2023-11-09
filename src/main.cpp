#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>
#include "main.hpp"
#include "wifi_secret.h"
#include "wireless_controller.hpp"


//AsyncClient* tcp_client = new AsyncClient;
//WiFiClient tcp_client;
WiFiUDP udp;
Ticker wireless_connection_ticker;
WirelessController controller;

String ip_addr;

bool safe_mode = false;


void setup() {
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    pinMode(15, OUTPUT);
    pinMode(0, INPUT_PULLUP);
    Serial.begin(921600);
    // rx:5 tx:7
    Serial1.begin(921600, SERIAL_8N1, 5, 3);
    
    WiFi.hostname("IMU_data_collector");
    
    Serial.println("boot...\n");

    // safe mode detect
    uint32_t boot_it_start = millis();
    uint32_t press_count = 0;
    while(millis() - boot_it_start < 3000) {
        if(!digitalRead(0)) {
            press_count++;
            delay(10);
        }

        if(press_count > 10) {
            safe_mode = true;
            Serial.print("Enter Safe Mode: ");
            Serial.println(press_count);
            break;
        }
    }

    if(!safe_mode) {
        Serial.println("start task\n");

        xTaskCreatePinnedToCore(
            droneSerialRxHandler,     /* Function to implement the task */
            "serialRx",         /* Name of the task */
            4000,                   /* Stack size in words */
            NULL,                   /* Task input parameter */
            1,                      /* Priority of the task */
            NULL,                   /* Task handle. */
            0                       /* Core where the task should run */
        );

        xTaskCreatePinnedToCore(
            droneSerialTxHandler,     /* Function to implement the task */
            "serialTx",         /* Name of the task */
            3000,                   /* Stack size in words */
            NULL,                   /* Task input parameter */
            0,                      /* Priority of the task */
            NULL,                   /* Task handle. */
            0                       /* Core where the task should run */
        );

        xTaskCreatePinnedToCore(
            wirelessHandler,    /* Function to implement the task */
            "wireless",       /* Name of the task */
            4000,           /* Stack size in words */
            NULL,           /* Task input parameter */
            1,              /* Priority of the task */
            NULL,           /* Task handle. */
            0               /* Core where the task should run */
        );

        Serial.println("end setup");
    }
    
}



void loop() {
    if(safe_mode) {
        static uint32_t last_change = 0;
        if(millis() - last_change > 200) {
            last_change = millis();
            digitalWrite(15, digitalRead(15)?0:1);
            Serial.print("Safe Mode Running: ");
            Serial.println(millis());
        }

    }else {
        static uint32_t last_change = 0;
        if(millis() - last_change > 1000) {
            last_change = millis();
            digitalWrite(15, digitalRead(15)?0:1);
        }

        controller.connectionWatchdog();
    }

    delay(1);
}




void droneSerialRxHandler(void* pvParameters) {
    uint16_t cnt;
    uint32_t last_print_time = 0;
    char buffer[300] = {'\0'};
    while(1) {
        int fifo_data_len = Serial1.available();
        //Serial.printf("drone rx len %d\n", fifo_data_len);
        
        // fifo available
        if(fifo_data_len > 0) {
            //Serial.printf("drone rx len %d\n", fifo_data_len);
            Serial1.readBytes(buffer, fifo_data_len);
            buffer[fifo_data_len] = '\0';
            String rx_str = String(buffer);
            //Serial.printf("serial rx: [%s]\n", rx_str.c_str());
            controller.rxProcess(rx_str, WirelessController::Drone);

            cnt ++;
            if(millis() - last_print_time >= 1000) {
                Serial.printf("drone rx cnt: %u\n", cnt);
                last_print_time = millis();
                cnt = 0;
            }
        // fifo empty
        }else {

        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}



void droneSerialTxHandler(void* pvParameters) {
    while(1) {
        String tx_str = controller.getTxPack(WirelessController::Drone);
        int tx_str_len = tx_str.length();
        if(tx_str_len > 0) {
            Serial1.write((uint8_t*)tx_str.c_str(), tx_str_len);
        }
        
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}








void wirelessHandler(void* pvParameters) {
    uint16_t cnt;
    uint32_t last_print_time = 0;
    char rx_buf[300] = {'\0'};
    while(1) {
        //Serial.printf("wrx %d\n", tcp_client.connected());
        // connect to wifi
        if(WiFi.status() != WL_CONNECTED) {
            while(WiFi.status() != WL_CONNECTED) {
                Serial.printf("Wifi Connecting...\n");
                WiFi.begin(SSID, PASSWORD);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
            Serial.printf("Wifi Connected\n");
            udp.begin(UDP_PORT);

        // udp io
        }else {
            int pack_size = udp.parsePacket();
            // receive client data
            if(pack_size > 0 && pack_size < 300) {
                // [receive data]
                udp.readBytes(rx_buf, pack_size);
                rx_buf[pack_size] = '\0';
                String rx_str = rx_buf;
                controller.rxProcess(rx_str, WirelessController::Host);

                // [transmit data]
                String tx_buffer = controller.getTxPack(WirelessController::Host);
                if(tx_buffer != "") {
                    udp.beginPacket(udp.remoteIP(), udp.remotePort());
                    udp.write((uint8_t*)tx_buffer.c_str(), tx_buffer.length());
                    udp.endPacket();
                }

                cnt ++;
                if(millis() - last_print_time >= 1000) {
                    Serial.printf("wireless rx/tx cnt: %u\n", cnt);
                    last_print_time = millis();
                    cnt = 0;
                }

            }else if(pack_size >= 300) {
                udp.flush();
                Serial.printf("rx too big\n");
            }
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}




