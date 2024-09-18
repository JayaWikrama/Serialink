/*
 * $Id: virtal-proxy.hpp,v 1.0.0 2024/09/18 12:56:03 Jaya Wikrama Exp $
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
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pty.h>
#include <cstring>
#include "virtual-proxy.hpp"

/**
 * @brief Default constructor.
 *
 * Initializes a virtual serial proxy port.
 *
 * Default parameters:
 * - Baud rate: B9600
 * - Timeout: 10 (1 second)
 * - Keep-alive interval: 10 milliseconds
 */
VirtualSerialProxy::VirtualSerialProxy(){
  this->workingBaudrate = B9600;
  this->physicalPort = "";
  this->symlinkPort = "";
  this->pty = new VirtualSerial(this->workingBaudrate, 10, 10);
  this->dev = new Serial(this->physicalPort, this->workingBaudrate, 10, 10);
  this->passthroughFunc = nullptr;
  this->passthroughParam = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * Initializes a virtual serial port with specified settings. The name or address of the virtual port for the slave
 * can be obtained using the `getVirtualPortName` method.
 *
 * @param physicalPort The connected physical serial devices.
 * @param baud The baud rate for the serial communication.
 */
VirtualSerialProxy::VirtualSerialProxy(const char *physicalPort, speed_t baud){
  this->workingBaudrate = baud;
  this->physicalPort = std::string(physicalPort);
  this->symlinkPort = "";
  this->pty = new VirtualSerial(this->workingBaudrate, 10, 10);
  this->dev = new Serial(this->physicalPort, this->workingBaudrate, 10, 10);
  this->passthroughFunc = nullptr;
  this->passthroughParam = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * Initializes a virtual serial port with specified settings. The name or address of the virtual port for the slave
 * can be obtained using the `getVirtualPortName` method.
 *
 * @param physicalPort The connected physical serial devices.
 * @param symlinkPort The symbolic link name of pseudo serial (may need to be run as root or require root/sudo password).
 * @param baud The baud rate for the serial communication.
 */
VirtualSerialProxy::VirtualSerialProxy(const char *physicalPort, const char *symlinkPort, speed_t baud){
  this->workingBaudrate = baud;
  this->physicalPort = std::string(physicalPort);
  this->symlinkPort = std::string(symlinkPort);
  this->pty = new VirtualSerial(this->workingBaudrate, 10, 10);
  this->dev = new Serial(this->physicalPort, this->workingBaudrate, 10, 10);
  this->passthroughFunc = nullptr;
  this->passthroughParam = nullptr;
}

/**
 * @brief Destructor.
 *
 * Releases all allocated memory and closes the master port of the virtual serial port.
 */
VirtualSerialProxy::~VirtualSerialProxy(){
  delete this->pty;
  delete this->dev;
}

/**
 * @brief Sets the physical serial port device.
 *
 * This setter function configures the serial port device to be used for communication.
 *
 * @param port The serial port device (e.g., "/dev/ttyUSB0").
 */
void VirtualSerialProxy::setPhysicalPort(const std::string port){
  this->dev->setPort(port);
  this->physicalPort = port;
}

/**
 * @brief Sets the symbolic link of pseudo serial port device.
 *
 * This setter function configures the serial port device to be used for communication.
 *
 * @param port The serial port device (e.g., "/dev/ttyS4").
 */
void VirtualSerialProxy::setSymlinkPort(const std::string port){
  this->symlinkPort = port;
}

/**
 * @brief Sets the baud rate for communication.
 *
 * This setter function configures the baud rate used for serial communication.
 *
 * @param baud The baud rate (e.g., `B9600` for 9600 bps).
 */
void VirtualSerialProxy::setBaudrate(speed_t baud){
  this->pty->setBaudrate(baud);
  this->dev->setBaudrate(baud);
  this->workingBaudrate = baud;
}

/**
 * @brief Sets the communication timeout.
 *
 * This setter function configures the timeout for serial communication. The timeout value is specified in units of 100 milliseconds.
 *
 * @param timeout The timeout value (e.g., `10` for a 1-second timeout).
 */
void VirtualSerialProxy::setTimeout(unsigned int timeout){
  this->pty->setTimeout(timeout);
  this->dev->setTimeout(timeout);
}

/**
 * @brief Sets the keep-alive interval for communication.
 *
 * This setter function configures the maximum wait time for receiving the next byte of serial data after the initial byte has been received. This helps maintain the connection by ensuring timely data reception.
 *
 * @param keepAliveMs The keep-alive interval in milliseconds.
 */
void VirtualSerialProxy::setKeepAlive(unsigned int keepAliveMs){
  this->pty->setKeepAlive(keepAliveMs);
  this->dev->setKeepAlive(keepAliveMs);
}

/**
 * @brief Gets the physical serial port.
 *
 * This getter function retrieves the physical serial port currently being used.
 *
 * @return The physical serial port as a string (e.g., "/dev/ttyUSB0").
 */
std::string VirtualSerialProxy::getPhysicalPort(){
  return this->physicalPort;
}

/**
 * @brief Gets the symbolic name of pseudo serial port.
 *
 * This getter function retrieves the physical serial port currently being used.
 *
 * @return The symbolic name of pseudo serial port (e.g., "/dev/ttyS4") of the name of pseudo serial port (if fail to set symbolic name or symbolic name is not set).
 */
std::string VirtualSerialProxy::getSymlinkPort(){
  return this->symlinkPort;
}

/**
 * @brief Gets the baud rate for serial communication.
 *
 * This getter function retrieves the baud rate currently configured for serial communication.
 *
 * @return The baud rate (e.g., `B9600` for 9600 bps).
 */
speed_t VirtualSerialProxy::getBaudrate(){
  return this->workingBaudrate;
}

/**
 * @brief Gets the communication timeout.
 *
 * This getter function retrieves the timeout value configured for serial communication. The timeout is specified in units of 100 milliseconds.
 *
 * @return The timeout value (e.g., `10` for a 1-second timeout).
 */
unsigned int VirtualSerialProxy::getTimeout(){
  return this->dev->getTimeout();
}

/**
 * @brief Gets the keep-alive interval for communication.
 *
 * This getter function retrieves the maximum wait time configured for receiving the next byte of serial data after the initial byte has been successfully received. The interval is specified in milliseconds.
 *
 * @return The keep-alive interval in milliseconds.
 */
unsigned int VirtualSerialProxy::getKeepAlive(){
  return this->dev->getKeepAlive();
}

/**
 * @brief Sets the Pass Through function.
 *
 * This method allows you to specify a callback function that will be used for handling data operations
 * (reading and writing) on the virtual serial poxy. (Mandatory before call the `begin` method).
 *
 * @param func Pass Through function that has 3 parameters. First `Serial &` is source and the second `Serial &` is destination. `void *` is a pointer that will connect directly to `void *param`.
 * @param param Pointer to the parameter for the callback function.
 */
void VirtualSerialProxy::setPassThrough(void (*func)(Serial &, Serial &, void *), void *param){
  this->passthroughFunc = (const void *) func;
  this->passthroughParam = param;
}

/**
 * @brief Gets the Pass Through function.
 *
 * @return Pointer to the callback function.
 */
const void *VirtualSerialProxy::getPassThroughFunction(){
  return this->passthroughFunc;
}

/**
 * @brief Gets the parameter pointer used for the Pass Through function.
 *
 * @return Pointer to the parameter of the Pass Through function.
 */
void *VirtualSerialProxy::getPassThroughParam(){
  return this->passthroughParam;
}

/**
 * @brief Method to start the proxy.
 *
 * @return `false` if the Pass Through function has not been set up or failed to start operation.
 * @return `true` if the Pass Through function has been successfully executed.
 */
bool VirtualSerialProxy::begin(){
  if (this->passthroughFunc == nullptr) return false;
  void (*callback)(Serial &, Serial &, void *) = (void (*)(Serial &, Serial &, void *))this->passthroughFunc;
  if (this->dev->openPort() != 0){
    std::cout << "Failed to open " << this->dev->getPort() << std::endl;
    return false;
  }
  if (this->symlinkPort.length() > 0){
    std::string cmd = std::string("sudo ln -s ") + this->pty->getVirtualPortName() + std::string(" ") + this->symlinkPort;
    if (system(cmd.c_str()) != 0){
      std::cout << "Linking " << this->pty->getVirtualPortName() << " to " << this->symlinkPort << " failed!" << std::endl;
      this->dev->closePort();
    }
  }
  fd_set readfds;
  int max = 0;
  int ret = 0;
  struct timeval tv;
  while (true) {
    tv.tv_sec = 1;
    tv.tv_usec = 500000;
    FD_ZERO(&readfds);
    if (this->dev->getFileDescriptor() > 0) FD_SET(this->dev->getFileDescriptor(), &readfds);
    if (this->pty->getFileDescriptor() > 0) FD_SET(this->pty->getFileDescriptor(), &readfds);
    max = (this->dev->getFileDescriptor() > this->pty->getFileDescriptor() ? this->dev->getFileDescriptor() : this->pty->getFileDescriptor());
    ret = select(max + 1 , &readfds , NULL , NULL , &tv);
    if (ret >= 0){
      if (FD_ISSET(this->dev->getFileDescriptor(), &readfds)){
        callback(*(this->dev), *(this->pty), this->passthroughParam);
      }
      else if (FD_ISSET(this->pty->getFileDescriptor(), &readfds)){
        callback(*(this->pty), *(this->dev), this->passthroughParam);
      }
    }
    else {
      usleep(125000);
    }
  }
  return true;
}