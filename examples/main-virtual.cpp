#include <iostream>
#include <string.h>
#include <unistd.h>
#include "virtuser.hpp"

void callbackEcho(VirtualSerial &ser, void *param){
    unsigned char buffer[1024];
    size_t sz = 0;
    int ret = 0;
    std::cout << __func__ << ": start (" << ser.getVirtualPortName() << ")" << std::endl;
    while (1){
        ret = ser.readData();
        if (ret == 0){
            sz = ser.getBuffer(buffer, sizeof(buffer) - 1);
            std::cout << "Received: " << std::to_string(sz) << " bytes" << std::endl;
            if (sz > 0){
                buffer[sz] = 0x00;
                std::cout << (const char *) buffer << std::endl;
                ser.writeData((const unsigned char *) "\x10\x02\x0A\x05\x19\x00\x00\x00\x00\x00\x09\x00\x04\x00\x28\x04\xAF\x3D\xE7\x52\x10\x10\x10\x03", 24);
            }
        }
        else {
            std::cout << "timeout: " << std::to_string(ret) << std::endl;
            sleep(1);
        }
    }
    std::cout << __func__ << ": end" << std::endl;
}

int main(int argc, char **argv){
    if (argc != 3){
        std::cout << "cmd: " << argv[0] << " <timeout100ms> <keepAliveMs>" << std::endl;
        exit(0);
    }
    VirtualSerial serial(B38400, atoi(argv[1]), atoi(argv[2]));
    serial.setCallback((const void *) &callbackEcho, nullptr);
    serial.begin();
    return 0;
}
