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
    /* test start bytes */
    while (serial.readStartBytes((const unsigned char *) "\x31\x32\x33", 3)){
        std::cout << "timeout!!!" << std::endl;
    }
    char data[512];
    memset(data, 0x00, sizeof(data));
    serial.getBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Data: " << data << std::endl;
    memset(data, 0x00, sizeof(data));
    serial.getRemainingBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Remaining Data: " << data << std::endl;
    /* test N Bytes Length */
    while (serial.readNBytes(24)){
        std::cout << "timeout!!!" << std::endl;
    }
    memset(data, 0x00, sizeof(data));
    serial.getBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Data: " << data << std::endl;
    memset(data, 0x00, sizeof(data));
    serial.getRemainingBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Remaining Data: " << data << std::endl;
    /* test Read Until Stop Bytes */
    while (serial.readUntilStopBytes((const unsigned char *) "\x31\x32\x33\x34", 4)){
        std::cout << "timeout!!!" << std::endl;
    }
    memset(data, 0x00, sizeof(data));
    serial.getBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Data: " << data << std::endl;
    memset(data, 0x00, sizeof(data));
    serial.getRemainingBuffer((unsigned char *) data, sizeof(data));
    std::cout << "Remaining Data: " << data << std::endl;
    serial.closePort();
    return 0;
}
