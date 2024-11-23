#include <iostream>
#include <string.h>
#include <unistd.h>

#include "usb-serial.hpp"

int main(int argc, char **argv){
    /* This Example For: Bus 003 Device 009: ID 0557:2008 ATEN International Co., Ltd UC-232A Serial Port [pl2303] */
    USBSerial usbSer( 0x0557, 0x2008, 0x83, 0x02, 0x20, 0x22, 38400, 1000);
    if (usbSer.openDevice()){
        exit(0);
    }
    size_t ret = 0;
    unsigned char data[128];
    while (true){
        ret = usbSer.readDevice(data, sizeof(data));
        if (ret > 0){
            printf("Data Diterima: ");
            for (int i = 0; i < ret; i++){
                printf("%02X ", data[i]);
            }
            printf("\n");
        }
        else {
            printf("Tidak ada data yang diterima!\n");
        }
        usbSer.writeDevice((const unsigned char *) "Waiting...\r\n", 12);
        printf("Waiting...\n");
    }
    usbSer.closeDevice();
    return 0;
}
