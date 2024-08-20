#include <iostream>
#include <string.h>

#include "serialink.hpp"

const std::string errorMessage[] = {
    "Serial Port Has Not Been Opened",
    "Timeout",
    "Frame Format Has Not Been Setup"
};

void setupLengthByCommand(DataFrame &frame, void *ptr){
    int data = 0;
    /* Get Serialink object form function param */
    Serialink *obj = (Serialink *) ptr;
    /* Get DataFrame target */
    DataFrame *target = (*obj)[DataFrame::FRAME_TYPE_DATA];
    if (target == nullptr) return;
    frame.getData((unsigned char *) &data, 1);
    if (data == 0x35){
        /* setup 3 as data size of DataFrame::FRAME_TYPE_DATA */
        target->setSize(3);
    }
    else if (data == 0x36){
        /* setup 3 as data size of DataFrame::FRAME_TYPE_DATA */
        target->setSize(2);
    }
    else {
        /* invalid value found -> trigger stop flag to __readFramedData__ method */
        obj->trigInvDataIndicator();
    }
}

void serialSetupDataFrameProtocol(Serialink &serial){
    /* Frame Data Format / Protocol Example
     * | Start  Bytes |   Command   |  Main Data   | Stop Bytes |
     * |:-------------|:------------|:-------------|:-----------|
     * |    4 bytes   |   1 byte    |   N bytes    | 4 bytes    |
     * |  0x31323334  | 0x35 / 0x36 | based on Cmd | 0x39302D3D |
     *
     * If Command = 0x35, then the Main Data length is 3 bytes.
     * If Command = 0x36, then the Main Data length is 2 bytes.
     * If Command is not equal to 0x35 and 0x36, then the data is invalid.
     */
    /* Configure Frame Format */
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    /* Setup the handler function to determine the data length of DataFrame::FRAME_TYPE_DATA.
     * This function is called after all data from DataFrame::FRAME_TYPE_COMMAND is received.
     */
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, (void *) &serial);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    /* Setup Frame Format to serial com */
    serial = startBytes + cmdBytes + dataBytes + stopBytes;
}

void displayData(const std::vector <unsigned char> &data){
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << (int) *i;
        std::cout << " ";
    }
    std::cout << std::endl;
}

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
    serialSetupDataFrameProtocol(serial);
    /* Do serial communication */
    serial.openPort();
    while ((ret = serial.readFramedData()) != 0){
        if (ret == 4){
            std::cout << "Invalid Received Data Details:" << std::endl;
            if (serial.getBuffer(data) > 0){
                std::cout << "    Received Data: ";
                displayData(data);
            }
            if (serial.getRemainingBuffer(data) > 0){
                std::cout << "    Remaining Received Data: ";
                displayData(data);
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
    displayData(data);
    return 0;
}
