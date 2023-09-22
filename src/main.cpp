#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <Ticker.h>
#include "main.hpp"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <WiFiClient.h>
#include "wifi_secret.h"


//AsyncClient* tcp_client = new AsyncClient;
WiFiClient tcp_client;
Ticker wireless_connection_ticker;

String ip_addr;
imu_data_t imu_buffer_0[BUF_0_COUNT];
imu_data_t imu_buffer_1[BUF_1_COUNT];

BufferHelper buf_help_0(imu_buffer_0, BUF_0_COUNT, 3, 0);
BufferHelper buf_help_1(imu_buffer_1, BUF_1_COUNT, 3, 0);

char tcp_pack_sample[100] = {'\0'};
size_t tcp_pack_len;

target_attitude_t target_attitude = {{0x7068, 2, 21}, 0, 0, 0};



void setup() {
    //WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
    clearBuffer(0);
    clearBuffer(1);

    sprintf(tcp_pack_sample, "phca%03u%05d%05d% 07d% 07d% 07d% 07d",
        38,
        1,
        2, 
        (int32_t)(0.3 * 1e5),
        (int32_t)(0.4 * 1e5),
        (int32_t)(0.5 * 1e5),
        (int32_t)(0.6 * 1e5)
    );
    tcp_pack_len = strlen(tcp_pack_sample);

    Serial.begin(921600);
    Serial2.begin(921600);

    Serial.println("boot\n");
    WiFi.hostname("IMU_data_collector");


    wireless_connection_ticker.attach(1, wirelessConnectionHandler);


    Serial.println("start task\n");

    xTaskCreatePinnedToCore(
        droneSerialRxHandler,     /* Function to implement the task */
        "serialRx",         /* Name of the task */
        5000,                   /* Stack size in words */
        NULL,                   /* Task input parameter */
        1,                      /* Priority of the task */
        NULL,                   /* Task handle. */
        1                       /* Core where the task should run */
    );

    xTaskCreatePinnedToCore(
        droneSerialRxHandler,     /* Function to implement the task */
        "serialTx",         /* Name of the task */
        5000,                   /* Stack size in words */
        NULL,                   /* Task input parameter */
        0,                      /* Priority of the task */
        NULL,                   /* Task handle. */
        1                       /* Core where the task should run */
    );

    xTaskCreatePinnedToCore(
        wirelessTxHandler,    /* Function to implement the task */
        "wirelessTx",       /* Name of the task */
        5000,           /* Stack size in words */
        NULL,           /* Task input parameter */
        0,              /* Priority of the task */
        NULL,           /* Task handle. */
        0               /* Core where the task should run */
    );

    xTaskCreatePinnedToCore(
        wirelessRxHandler,    /* Function to implement the task */
        "wirelessRx",       /* Name of the task */
        5000,           /* Stack size in words */
        NULL,           /* Task input parameter */
        1,              /* Priority of the task */
        NULL,           /* Task handle. */
        0               /* Core where the task should run */
    );

    Serial.println("end setup");
}



void loop() {
    delay(1);
}




void droneSerialRxHandler(void* pvParameters) {
    while(1) {
        int fifo_data_len = Serial2.available();
        //Serial.printf("fifo data len %d\n", fifo_data_len);
        // fifo empty
        if(fifo_data_len == 0) {
        
        // fifo len fit
        }else if( fifo_data_len % sizeof(imu_data_t) == 0 ) {
            //Serial.printf("valid data\n");
            BufferHelper* buf_help_ptr = getWriteableBufferHelper();
            // find writable buffer
            if(buf_help_ptr != nullptr) {
                buf_help_ptr->setFlagWriting();
                size_t serial_data_count = fifo_data_len / sizeof(imu_data_t);
                size_t write_count = buf_help_ptr->getWriteableCount(serial_data_count);
                size_t write_index = buf_help_ptr->getWriteIndex();
                Serial2.readBytes((uint8_t*)&buf_help_ptr->getDataPtr<imu_data_t>()[write_index], fifo_data_len);
                buf_help_ptr->markWriteCount(write_count);

                /*
                size_t debug_index = buf_help_ptr->getWriteIndex() - 1;
                Serial.printf("b%d i%u f%d t%d w%f x%f y%f z%f\n",
                    buf_help_ptr->buffer_id,
                    debug_index,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].frame,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].dt,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].qw,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].qx,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].qy,
                    buf_help_ptr->getDataPtr<imu_data_t>()[debug_index].qz
                );
                */
                
            }else {
                //Serial.printf("no buffer can write\n");
            }
            //Serial.printf("buf0 R %u W %u\n", buf_help_0.getReadIndex(), buf_help_0.getWriteIndex());
            //Serial.printf("buf1 R %u W %u\n", buf_help_1.getReadIndex(), buf_help_1.getWriteIndex());
            

        // fifo len is garbage
        }else {
            uint8_t garbage[255];
            Serial2.readBytes(garbage, fifo_data_len);
            Serial.printf("Wrong input len %d\n", fifo_data_len);
        }

        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}



void droneSerialTxHandler(void* pvParameters) {
    while(1) {
        if(tcp_client.connected()) {
            int fifo_len = Serial2.availableForWrite();

            if(fifo_len > 50) {
                Serial2.write((uint8_t*)&target_attitude, sizeof(target_attitude_t));
            }
        }
        vTaskDelay(20 / portTICK_PERIOD_MS);
    }
}





void wirelessTxHandler(void* pvParameters) {
    while(1) {
        // if socket can send
        //if(tcp_client->canSend()) {
        if(tcp_client.connected()) {
            BufferHelper* buf_help_ptr = getReadableBufferHelper();
            // find readable buffer
            if(buf_help_ptr != nullptr) {
                buf_help_ptr->setFlagReading();
                //size_t fifo_left_len = tcp_client->space();
                //size_t fifo_left_count = fifo_left_len / tcp_pack_len;
                size_t fifo_left_count = 100;
                size_t read_count = buf_help_ptr->getReadableCount(fifo_left_count);
                size_t read_index = buf_help_ptr->getReadIndex();
                
                //Serial.printf("fifo left %d count %d\n", fifo_left_len, fifo_left_count);
                //Serial.printf("will send buf %d count %d\n", buf_help_ptr->buffer_id, read_count);
                

                char buf[100] = {'\0'};
                //Serial.printf("send pack\n");
                for(size_t i = read_index; i < read_count + read_index; i++) {
                    memset(buf, '\0', 100);
                    sprintf(buf, "phca%03u%05d%05d% 07d% 07d% 07d% 07d",
                        38,
                        buf_help_ptr->getDataPtr<imu_data_t>()[i].frame,
                        buf_help_ptr->getDataPtr<imu_data_t>()[i].dt, 
                        (int32_t)(buf_help_ptr->getDataPtr<imu_data_t>()[i].qw * 1e5),
                        (int32_t)(buf_help_ptr->getDataPtr<imu_data_t>()[i].qx * 1e5),
                        (int32_t)(buf_help_ptr->getDataPtr<imu_data_t>()[i].qy * 1e5),
                        (int32_t)(buf_help_ptr->getDataPtr<imu_data_t>()[i].qz * 1e5)
                    );
                    //Serial.printf("frame: %u\n", buf_help_ptr->getDataPtr<imu_data_t>()[i].frame);
                    //Serial.printf("%d %d %s", tcp_pack_len, strlen(buf), buf);
                    //Serial.printf("%d %d %s", tcp_pack_len, strlen(buf), tcp_pack_sample);
                    //tcp_client->add(buf, tcp_pack_len);
                    tcp_client.print(buf);
                }
                //Serial.printf("fifo left %d count %d\n", tcp_client->space(), tcp_client->space() / tcp_pack_len);
                //tcp_client->send();
                buf_help_ptr->markReadCount(read_count);
            }
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}




void wirelessRxHandler(void* pvParameters) {
    uint8_t rx_buf[200] = {0};
    while(1) {
        if(tcp_client.connected()) {
            //static size_t data_len = 65535;
            size_t rx_len = 0;
            while(rx_len == 0) {
                rx_len = tcp_client.available();
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            //Serial.println(rx_len);

            tcp_client.read((uint8_t*)rx_buf, rx_len);
            String rx_buf_str = (char*)rx_buf;
            size_t buf_str_len = rx_buf_str.length();
            //Serial.printf("[Raw][%u]",buf_str_len);
            //Serial.println(rx_buf_str);
            size_t start_idx = 0;
            while(buf_str_len - start_idx >= 7) {
                int header_idx = rx_buf_str.indexOf("ph", start_idx);
                //Serial.printf("[h_idx]%d\n", header_idx);
                if(header_idx == -1) {
                    //Serial.println("end buf");
                    break;
                }
                String pack_type = rx_buf_str.substring(header_idx+2, header_idx+4);
                //Serial.println((size_t)&pack_type);

                size_t data_len = rx_buf_str.substring(header_idx+4, header_idx+7).toInt();
                //Serial.printf("[type]%s[d_len]%u\n", pack_type, data_len);

                if(data_len == 0) {
                    start_idx = header_idx+7;
                    //Serial.println("no data");
                    continue;
                }else {
                    start_idx = header_idx+7+data_len;
                }

                String target_attitude_str = rx_buf_str.substring(header_idx+7, header_idx+7+data_len);
                int32_t pitch = target_attitude_str.substring(0, 7).toInt();
                int32_t roll = target_attitude_str.substring(7, 14).toInt();
                int32_t yaw = target_attitude_str.substring(14, 21).toInt();
                target_attitude.pitch = pitch;
                target_attitude.roll = roll;
                target_attitude.yaw = yaw;
                //Serial.println(data);
                //Serial.printf("[start_idx at end]%u\n", start_idx);
            }
            

        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}




void wirelessConnectionHandler() {
    //Serial.printf("[wirelessConnectionHandler]\n");
    if(WiFi.status() != WL_CONNECTED) {
        Serial.printf("Wifi Connecting...\n");
        WiFi.begin(SSID, PASSWORD);
    }else if(!tcp_client.connected()) {
        Serial.printf("Server Connecting...\n");
        tcp_client.connect(SERVER_IP, SERVER_PORT, 500);
        if(tcp_client.connected()) {
            Serial.printf("Server Connected\n");
            tcp_client.setNoDelay(true);
        }
    //}else if(!tcp_client->connected()) {
    //    Serial.printf("Server Connecting...\n");
    //    tcp_client->connect(SERVER_IP, SERVER_PORT);
    }else {
        //Serial.printf("Connection OK\n");
    }
}



void clearBuffer(uint8_t buf_id){
    imu_data_t* buf_ptr;
    size_t buf_cnt;
    switch(buf_id){
        case 0: 
            buf_ptr = imu_buffer_0;
            buf_cnt = BUF_0_COUNT;
            break;
        case 1: 
            buf_ptr = imu_buffer_1;
            buf_cnt = BUF_1_COUNT;
            break;
        default:
            return;
    }

    for(size_t idx = 0; idx < buf_cnt; idx++){
        buf_ptr[idx].dt = 0;
        buf_ptr[idx].frame = 0;
        buf_ptr[idx].qw = 1.0f;
        buf_ptr[idx].qx = 0.0f;
        buf_ptr[idx].qy = 0.0f;
        buf_ptr[idx].qz = 0.0f;
    }
}




BufferHelper* getWriteableBufferHelper(){
    // use writing buffer first
    if(buf_help_0.isWriting()) {
        return &buf_help_0;
    }else if(buf_help_1.isWriting()) {
        return &buf_help_1;
    // than using empty buffer
    }else if(buf_help_0.isEmpty()) {
        return &buf_help_0;
    }else if(buf_help_1.isEmpty()) {
        return &buf_help_1;
    // not found
    }else {
        return nullptr;
    }
}



BufferHelper* getReadableBufferHelper(){
    // use writing buffer first
    if(buf_help_0.isReading()) {
        return &buf_help_0;
    }else if(buf_help_1.isReading()) {
        return &buf_help_1;
    // than using empty buffer
    }else if(buf_help_0.isFull()) {
        return &buf_help_0;
    }else if(buf_help_1.isFull()) {
        return &buf_help_1;
    // not found
    }else {
        return nullptr;
    }
}



