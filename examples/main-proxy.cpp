#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "virtual-proxy.hpp"

void displayData(const std::vector <unsigned char> &data){
    for (int i = 0; i < data.size(); i++){
        if (i == 12 | i == data.size() - 3) printf("| ");
        printf("%02X ", data[i]);
    }
    std::cout << std::endl;
}

void passthroughFunc(Serial &src, Serial &dest, void *param){
    std::vector <unsigned char> data;
    if (src.readData() == 0){
        data = src.getBufferAsVector();
        std::cout << src.getPort() << " >>> " << dest.getPort() << " [sz=" << std::to_string(data.size()) << "] : ";
        displayData(data);
        dest.writeData(data);
    }
}

int main(int argc, char **argv){
    if (argc != 2){
        std::cout << "cmd: " << argv[0] << " <physicalPort>" << std::endl;
        exit(0);
    }
    VirtualSerialProxy proxy(argv[1], B38400);
    proxy.setPassThrough(&passthroughFunc, nullptr);
    proxy.begin();
    return 0;
}
