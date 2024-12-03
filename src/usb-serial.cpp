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

#include <iostream>
#include "usb-serial.hpp"

#ifdef __USE_USB_SERIAL__
#define REQUEST_TYPE_OUT (LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_OUT)
#endif

/**
 * @brief Configures serial devices attributes.
 *
 * This function sets up or configures the attributes of the usb serial devices.
 *
 * @return true if the configuration is successful.
 * @return false if the configuration fails.
 */
bool USBSerial::setupAttributes(){
#ifdef __USE_USB_SERIAL__
  struct lineCoding {
    unsigned int baud;
    unsigned char stopBits;
    unsigned char parity;
    unsigned char dataBits;
  };
  struct lineCoding coding = {
    .baud = this->baudrate,
    .stopBits = 0,
    .parity = 0,
    .dataBits = 8
  };
  int ret = libusb_control_transfer(this->handle, REQUEST_TYPE_OUT, this->requestSetLineCoding,
                                    0, 0, (unsigned char *)&coding, sizeof(coding), this->timeout);
  if (ret < 0) {
    std::cout << __func__ << ": Failed to set baud rate: " << libusb_error_name(ret) << std::endl;
    return false;
  }

  ret = libusb_control_transfer(this->handle, REQUEST_TYPE_OUT, this->requestSetControlLinestate,
                                 0x0003, 0, NULL, 0, this->timeout);
  if (ret < 0) {
    std::cout << __func__ << ": Failed to set control line state: " << libusb_error_name(ret) << std::endl;
    return false;
  }
  return true;
#else
  std::cout << __func__ << ": USB Serial Feature is disabled!" << std::endl;
  return false;
#endif
}

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
USBSerial::USBSerial(
 unsigned short vendorID,
 unsigned short productID,
 unsigned char endPointIn,
 unsigned char endPointOut,
 unsigned char requestSetLineCoding,
 unsigned char requestSetControlLinestate,
 unsigned int baudrate,
 unsigned short timeout
){
  this->vendorID = vendorID;
  this->productID = productID;
  this->endPointIn = endPointIn;
  this->endPointOut = endPointOut;
  this->requestSetLineCoding = requestSetLineCoding;
  this->requestSetControlLinestate = requestSetControlLinestate;
  this->baudrate = baudrate;
  this->timeout = timeout;
#ifdef __USE_USB_SERIAL__
  this->ctx = nullptr;
  this->handle = nullptr;
#endif
}

/**
 * @brief Destructor.
 *
 * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
 * It ensures that all allocated resources are properly freed, preventing memory leaks.
 */
USBSerial::~USBSerial(){
#ifdef __USE_USB_SERIAL__
  if (this->handle) this->closeDevice();
  if (this->ctx){
    libusb_exit(this->ctx);
    this->ctx = nullptr;
  }
#endif
}

/**
 * @brief Opens the usb serial device for communication.
 *
 * This function attempts to open the specified usb serial device for communication.
 *
 * @return 0 if the device is successfully opened.
 * @return 1 if the device fails to open.
 */
int USBSerial::openDevice(){
#ifdef __USE_USB_SERIAL__
  int ret = 0;
  if (this->ctx == nullptr){
    ret = libusb_init(&(this->ctx));
    if (ret < 0) {
      std::cout << __func__ << ": Failed to initialize libusb: " << libusb_error_name(ret) << std::endl;
      return 1;
    }
  }
  this->handle = libusb_open_device_with_vid_pid(this->ctx, this->vendorID, this->productID);
  if (this->handle == nullptr) {
    std::cout << __func__ << ": Failed to open devices ";
    printf("%04X:%04X\n", this->vendorID, this->productID);
    return 1;
  }
  ret = libusb_claim_interface(handle, 0);
  if (ret < 0) {
    std::cout << __func__ << ": Failed to claim interface: " << libusb_error_name(ret) << std::endl;
    libusb_close(this->handle);
    this->handle = nullptr;
    return 1;
  }
  if (this->setupAttributes() == false){
    libusb_close(this->handle);
    this->handle = nullptr;
  }
  return 0;
#else
  std::cout << __func__ << ": USB Serial Feature is disabled!" << std::endl;
  return 1;
#endif
}

/**
 * @brief Performs a serial data read operation.
 *
 * This function reads data from the serial port without separating the successfully read data into the desired size and remaining data. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param[out] buffer All data that is successfully read will be stored in this variable.
 * @param[in] sz The number of bytes to read. A value of `0` means that the read operation is unlimited (up to the `keepAliveMs` timeout).
 * @return The total data successfully received.
 */
size_t USBSerial::readDevice(unsigned char *buffer, size_t sz){
#ifdef __USE_USB_SERIAL__
  int transferred = 0;
  int ret = 0;
  unsigned short partTimeout = 25;
  size_t total = 0;
  bool tryFinished = false;
  ret = libusb_bulk_transfer(this->handle, this->endPointIn, buffer, sz, &transferred, this->timeout);
  if (ret == LIBUSB_SUCCESS){
    total += static_cast<size_t>(transferred);
    while (total < sz){
      ret = libusb_bulk_transfer(this->handle, this->endPointIn, buffer + total, sz - total, &transferred, partTimeout);
      if (ret == LIBUSB_SUCCESS){
        total += static_cast<size_t>(transferred);
        tryFinished = false;
      }
      else {
        if (tryFinished == false) tryFinished = true;
        else {
          break;
        }
      }
    }
  }
  if (total == 0){
    std::cout << __func__ << ": Failed to read data: " << libusb_error_name(ret) << std::endl;
  }
  return total;
#else
  std::cout << __func__ << ": USB Serial Feature is disabled!" << std::endl;
  return 0;
#endif
}

/**
 * @brief Performs the operation of writing serial data.
 *
 * This method writes the specified data to the serial port.
 *
 * @param[in] buffer Data to be written.
 * @param[in] sz Size of the data to be written.
 * @return The total data successfully send.
 */
size_t USBSerial::writeDevice(const unsigned char *buffer, size_t sz){
#ifdef __USE_USB_SERIAL__
  int transferred = 0;
  int ret = 0;
  unsigned short partTimeout = 25;
  size_t total = 0;
  bool tryFinished = false;
  ret = libusb_bulk_transfer(this->handle, this->endPointOut, (unsigned char *) buffer, sz, &transferred, this->timeout);
  if (ret == LIBUSB_SUCCESS){
    total += static_cast<size_t>(transferred);
    while (total < sz){
      ret = libusb_bulk_transfer(this->handle, this->endPointOut, (unsigned char *) buffer + total, sz - total, &transferred, partTimeout);
      if (ret == LIBUSB_SUCCESS){
        total += static_cast<size_t>(transferred);
        tryFinished = false;
      }
      else {
        if (tryFinished == false) tryFinished = true;
        else {
          std::cout << __func__ << ": Failed to write data: " << libusb_error_name(ret) << std::endl;
          break;
        }
      }
    }
  }
  return total;
#else
  std::cout << __func__ << ": USB Serial Feature is disabled!" << std::endl;
  return 0;
#endif
}

/**
 * @brief Closes the usb serial device.
 *
 * This function is used to close the usb serial devices, ensuring that the port is no longer in use and that any associated system resources
 * are released.
 */
void USBSerial::closeDevice(){
#ifdef __USE_USB_SERIAL__
  if (this->handle != nullptr){
    libusb_close(this->handle);
    this->handle = nullptr;
  }
#else
  std::cout << __func__ << ": USB Serial Feature is disabled!" << std::endl;
#endif
}
