#include <iostream>
#include <string.h>

#include "serialink.hpp"
#include "data-formating.hpp"

const std::string errorMessage[] = {
    "Serial Port Has Not Been Opened",
    "Timeout",
    "Frame Format Has Not Been Setup"
};

int main(int argc, char **argv){
    if (argc != 4){
        std::cout << "cmd: " << argv[0] << " <port> <timeout100ms> <keepAliveMs>" << std::endl;
        exit(0);
    }
    int ret = 0;
    std::vector <unsigned char> data;
    Serialink serial;
    /* Prepare Object */
    serial.setPort(argv[1]);
    serial.setBaudrate(B115200);
    serial.setTimeout(atoi(argv[2]));
    serial.setKeepAlive(atoi(argv[3]));
    /* Setup Protocol */
    ProtocolFormat protocol(serial);
    /* Do serial communication */
    serial.openPort();
    /* Send some data */
    serial.writeData(protocol.buildCommand((const unsigned char *) "\x36\x37\x38", 3));
    /* Receive data */
    while ((ret = serial.readFramedData()) != 0){
        if (ret == 4){
            std::cout << "Invalid Received Data Details:" << std::endl;
            if (serial.getBuffer(data) > 0){
                std::cout << "    Received Data: ";
                ProtocolFormat::displayData(data);
            }
            if (serial.getRemainingBuffer(data) > 0){
                std::cout << "    Remaining Received Data: ";
                ProtocolFormat::displayData(data);
            }
            std::cout << std::endl;
        }
        else {
            std::cout << errorMessage[ret - 1] << std::endl;
        }
    }
    serial.closePort();
    serial.getFormat()->getAllData(data);
    std::cout << "Received Success [" + std::to_string(serial.getDataSize()) + "]" << std::endl;
    std::cout << "    Data: ";
    ProtocolFormat::displayData(data);
    return 0;
}
