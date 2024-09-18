#include <iostream>
#include <string.h>
#include <unistd.h>
#include "virtual-proxy.hpp"

void passthroughFunc(Serial &src, Serial &dest, void *param){
    std::vector <unsigned char> data;
    if (src.readData() == 0){
        data = src.getBufferAsVector();
        dest.writeData(data);
    }
}

int main(int argc, char **argv){
    if (argc != 2){
        std::cout << "cmd: " << argv[0] << " <physicalPort>" << std::endl;
        exit(0);
    }
    VirtualSerialProxy serial(argv[1], B9600);
    serial.setPassThrough(&passthroughFunc, nullptr);
    serial.begin();
    return 0;
}
