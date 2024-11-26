#include <iostream>
#include <string.h>
#include <unistd.h>

#include "serialink.hpp"

const std::string errorMessage[] = {
    "Serial Port Has Not Been Opened",
    "Timeout",
    "Frame Format Has Not Been Setup"
};

void displayData(const std::vector <unsigned char> &data){
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << (int) *i;
        std::cout << " ";
    }
    std::cout << std::endl;
}

void crc16(DataFrame &frame, void *ptr) {
    /* Get Serialink object form function param */
    Serialink *obj = (Serialink *) ptr;
    /* Initialize crc16 param */
    unsigned short crc = 0x0000;
    unsigned short poly = 0x1021;
    /* Get data from Start Bytes until Data */
    std::vector <unsigned char> data = obj->getSpecificBufferAsVector(DataFrame::FRAME_TYPE_START_BYTES, DataFrame::FRAME_TYPE_DATA);
    std::cout << "Data from which the CRC value will be calculated:" << std::endl;
    displayData(data);
    /* Calculate crc16 */
    for (const auto &byte : data) {
        crc ^= (static_cast<unsigned short>(byte) << 8);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }
    /* Compare received CRC with calculated CRC */
    unsigned short rcvCRC = 0;
    frame.getData((unsigned char *) &rcvCRC, 2);
    if (rcvCRC != crc){
        /* invalid crc -> trigger stop flag to __readFramedData__ method */
        obj->trigInvDataIndicator();
        std::cout << "CRC16 Invalid (0x" << std::hex << rcvCRC << " != 0x" << std::hex << crc << ")" << std::endl;
    }
}

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
     * | Start  Bytes |   Command   |  Main Data   | CRC16 Validator | Stop Bytes |
     * |:-------------|:------------|:-------------|:----------------|:-----------|
     * |    4 bytes   |   1 byte    |   N bytes    |     2 bytes     | 4 bytes    |
     * |  0x31323334  | 0x35 / 0x36 | based on Cmd |  init = 0x0000  | 0x39302D3D |
     *
     * If Command = 0x35, then the Main Data length is 3 bytes.
     * If Command = 0x36, then the Main Data length is 2 bytes.
     * If Command is not equal to 0x35 and 0x36, then the data is invalid.
     *
     * Use this input example: 3132333435363738159039302D3D
     */
    /* Configure Frame Format */
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    /* Setup the handler function to determine the data length of DataFrame::FRAME_TYPE_DATA.
     * This function is called after all data from DataFrame::FRAME_TYPE_COMMAND is received.
     */
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, (void *) &serial);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame crcValidatorBytes(DataFrame::FRAME_TYPE_VALIDATOR, 2);
    /* Setup the handler function to validate Data by using crc16 validation.
     * This function is called after all data from DataFrame::FRAME_TYPE_VALIDATOR is received.
     */
    crcValidatorBytes.setPostExecuteFunction((const void *) &crc16, (void *) &serial);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    /* Setup Frame Format to serial com */
    serial = startBytes + cmdBytes + dataBytes + crcValidatorBytes + stopBytes;
}

int main(int argc, char **argv){
    int ret = 0;
    std::vector <unsigned char> data;
    /* This Example For: Bus 003 Device 009: ID 0557:2008 ATEN International Co., Ltd UC-232A Serial Port [pl2303] */
    USBSerial *usbSer = new USBSerial( 0x0557, 0x2008, 0x83, 0x02, 0x20, 0x22, 38400, 1000);
    Serialink serial(usbSer);
    serialSetupDataFrameProtocol(serial);
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