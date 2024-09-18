#include <iostream>
#include <string.h>
#include <unistd.h>
#include "virtual-proxy.hpp"

void displayData(const std::vector <unsigned char> &data){
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << (int) *i;
        std::cout << " ";
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
    VirtualSerialProxy proxy(argv[1], B9600);
    proxy.setPassThrough(&passthroughFunc, nullptr);
    proxy.begin();
    return 0;
}
