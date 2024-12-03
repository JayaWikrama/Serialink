/*
 * $Id: usb-serial.hpp,v 1.0.0 2024/11/16 12:59:13 Jaya Wikrama Exp $
 *
 * Copyright (c) 2024 Jaya Wikrama
 * jayawikrama89@gmail.com
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

/**
 * @file
 * @brief Direct Access to USB for Serial Communication.
 *
 * This file contains a collection of functions and commands designed to
 * facilitate basic serial communications in C++ directly using a USB device
 * (without a Serial Driver)
 *
 * @note This file is a part of a larger project focusing on enhancing serial communication
 *       capabilities in C++ applications.
 *
 * @version 1.0.0
 * @date 2024-11-16
 * @author Jaya Wikrama
 */

#ifndef __USB_SERIAL_HPP__
#define __USB_SERIAL_HPP__

#ifdef __USE_USB_SERIAL__
#include <libusb-1.0/libusb.h>
#endif

class USBSerial {
  private:
    unsigned short vendorID;
    unsigned short productID;
    unsigned char endPointIn;
    unsigned char endPointOut;
    unsigned char requestSetLineCoding;
    unsigned char requestSetControlLinestate;
    unsigned short timeout;
    unsigned int baudrate;
#ifdef __USE_USB_SERIAL__
    libusb_context *ctx;
    libusb_device_handle *handle;
#endif
  
  protected:
    /**
     * @brief Configures serial devices attributes.
     *
     * This function sets up or configures the attributes of the usb serial devices.
     *
     * @return true if the configuration is successful.
     * @return false if the configuration fails.
     */
    bool setupAttributes();

  public:
    /**
     * @brief Default constructor.
     *
     * This constructor initializes private data members and parameters to specific value;
     * @param[in] vendorID You can get this value form `lsusb` command. (exp: ... 0557:2008 ATEN ... VID: 0x2008).
     * @param[in] productID You can get this value form `lsusb` command. (exp: ... 0557:2008 ATEN ... PID: 0x0557).
     * @param[in] endPointIn You can get this value from `lsusb -v -d <PID:VID>. (search bEndpointAddress ... IN with Bulk Transfer Type)
     * @param[in] endPointOut You can get this value from `lsusb -v -d <PID:VID>. (search bEndpointAddress ... OUT with Bulk Transfer Type)
     * @param[in] requestSetLineCoding Specific request types for line coding (default: 0x20)
     * @param[in] requestSetControlLinestate Specific request types for line coding (default: 0x22)
     * @param[in] baudrate Specific baudrate
     * @param[in] timeout reception timeout
     */
    USBSerial(
     unsigned short vendorID,
     unsigned short productID,
     unsigned char endPointIn,
     unsigned char endPointOut,
     unsigned char requestSetLineCoding,
     unsigned char requestSetControlLinestate,
     unsigned int baudrate,
     unsigned short timeout
    );

    /**
     * @brief Destructor.
     *
     * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
     * It ensures that all allocated resources are properly freed, preventing memory leaks.
     */
    ~USBSerial();

    /**
     * @brief Opens the usb serial device for communication.
     *
     * This function attempts to open the specified usb serial device for communication.
     *
     * @return 0 if the device is successfully opened.
     * @return 1 if the device fails to open.
     */
    int openDevice();

    /**
     * @brief Performs a serial data read operation.
     *
     * This function reads data from the serial port without separating the successfully read data into the desired size and remaining data. The read serial data can be accessed using the `Serial::getBuffer` method.
     *
     * @param[out] buffer All data that is successfully read will be stored in this variable.
     * @param[in] sz The number of bytes to read. A value of `0` means that the read operation is unlimited (up to the `keepAliveMs` timeout).
     * @return The total data successfully received.
     */
    size_t readDevice(unsigned char *buffer, size_t sz);

    /**
     * @brief Performs the operation of writing serial data.
     *
     * This method writes the specified data to the serial port.
     *
     * @param[in] buffer Data to be written.
     * @param[in] sz Size of the data to be written.
     * @return The total data successfully send.
     */
    size_t writeDevice(const unsigned char *buffer, size_t sz);

    /**
     * @brief Closes the usb serial device.
     *
     * This function is used to close the usb serial devices, ensuring that the port is no longer in use and that any associated system resources
     * are released.
     */
    void closeDevice();
};

#endif
