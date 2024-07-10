#include <iostream>
#include <string.h>

#include "serial.hpp"

int main(int argc, char **argv){
    if (argc != 4){
        std::cout << "cmd: " << argv[0] << " <port> <timeout100ms> <keepAliveMs>" << std::endl;
        exit(0);
    }
    Serial serial(argv[1], B115200, atoi(argv[2]));
    serial.openPort();
    serial.setKeepAlive(atoi(argv[3]));
    while (serial.readData()){
        std::cout << "timeout!!!" << std::endl;
    }
    char data[512];
    memset(data, 0x00, sizeof(data));
    serial.getBuffer((unsigned char *) data, sizeof(data));
    std::cout << data << std::endl;
    serial.closePort();
    return 0;
}
