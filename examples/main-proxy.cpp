#include <iostream>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iomanip>
#include "virtual-proxy.hpp"

static void displayData(const unsigned char *data, size_t sz){
    for (int i = 0; i < sz; i++) {
        std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)(data[i]);
        std::cout << " ";
    }
    std::cout << std::endl;
}

void passthroughFunc(Serial &src, Serial &dest, void *param){
    std::vector <unsigned char> data;
    if (src.readData() == 0){
        data = src.getBufferAsVector();
        std::cout << src.getPort() << " >>> " << dest.getPort() << " [sz=" << std::to_string(data.size()) << "] : ";
        displayData(data.data(), data.size());
        dest.writeData(data);
    }
}

int main(int argc, char **argv){
    if (argc != 2){
        std::cout << "cmd: " << argv[0] << " <physicalPort>" << std::endl;
        exit(0);
    }
    VirtualSerialProxy proxy(argv[1], B115200);
    proxy.setPassThrough(&passthroughFunc, nullptr);
    proxy.begin();
    return 0;
}
