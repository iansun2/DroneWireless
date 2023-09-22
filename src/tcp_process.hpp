#ifndef __TCP_PROCESS__
#define __TCP_PROCESS__

#include <Arduino.h>

class TcpProcess{
public:
    void receive(String& type, String& payload);

    String getSend();

private:
    String rx_pass_buffer;



};



#endif