#include <iostream>
#include <string.h>

#include "serialink.hpp"

void setupLengthByCommand(DataFrame &frame, void *ptr){
    int data = 0;
    DataFrame *target = frame.getNext();
    if (target == nullptr) return;
    frame.getData((unsigned char *) &data, 1);
    if (data == 0x35)
        target->setSize(3);
    else if (data == 0x36)
        target->setSize(2);
}

int main(int argc, char **argv){
    if (argc != 4){
        std::cout << "cmd: " << argv[0] << " <port> <timeout100ms> <keepAliveMs>" << std::endl;
        exit(0);
    }
    int ret = 0;
    Serialink serial;
    serial.setPort(argv[1]);
    serial.setBaudrate(B115200);
    serial.setTimeout(atoi(argv[2]));
    serial.setKeepAlive(atoi(argv[3]));
    /* Configure Frame Format */
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    /* Setup Frame Format to serial com */
    serial = startBytes + cmdBytes + dataBytes + stopBytes;
    /* Do serial communication */
    serial.openPort();
    while ((ret = serial.readFramedData()) != 0){
        std::cout << "err: " << ret << std::endl;
    }
    std::vector <unsigned char> data;
    serial.getFormat()->getAllData(data);
    std::cout << "Data size = " << data.size() << std::endl;
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << *i;
        std:: cout << " ";
    }
    return 0;
}
