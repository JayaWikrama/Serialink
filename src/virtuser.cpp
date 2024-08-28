/*
 * $Id: virtuser.cpp,v 1.0.0 2024/07/29 18:41:03 Jaya Wikrama Exp $
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
#include "virtuser.hpp"

/**
 * @brief Default constructor.
 *
 * Initializes a virtual serial port. The name or address of the virtual port for the slave
 * can be obtained using the `getVirtualPortName` method.
 *
 * Default parameters:
 * - Baud rate: B9600
 * - Timeout: 10 (1 second)
 * - Keep-alive interval: 0 milliseconds
 *
 * Initializes a mutex for thread safety.
 */
VirtualSerial::VirtualSerial(){
  int masterFd = -1;
  int slaveFd = -1;
  char virtualSerialName[128];
  this->setPort("");
  if (openpty(&masterFd, &slaveFd, virtualSerialName, NULL, NULL) == -1) {
    std::cerr << "Gagal membuat virtual serial port: " << strerror(errno) << std::endl;
    return;
  }
  std::cout << "Virtual Serial Name (slave): " << virtualSerialName << std::endl;
  this->virtualPortName = std::string(virtualSerialName);
  close(slaveFd);
  this->setFileDescriptor(masterFd);
  if (this->setupAttributes() == false){
    std::cerr << "Gagal melakukan setup attribut virtual serial port" << std::endl;
  }
  else {
    std::cerr << "Virtual serial port ready" << std::endl;
  }
}

/**
 * @brief Custom constructor.
 *
 * Initializes a virtual serial port with specified settings. The name or address of the virtual port for the slave
 * can be obtained using the `getVirtualPortName` method.
 *
 * @param baud The baud rate for the serial communication.
 * @param timeout The timeout period in 100 milliseconds.
 * @param keepAliveMs The keep-alive interval in milliseconds.
 */
VirtualSerial::VirtualSerial(speed_t baud, unsigned int timeout, unsigned int keepAliveMs){
  int masterFd = -1;
  int slaveFd = -1;
  char virtualSerialName[128];
  this->setPort("");
  if (openpty(&masterFd, &slaveFd, virtualSerialName, NULL, NULL) == -1) {
    std::cerr << "Gagal membuat virtual serial port: " << strerror(errno) << std::endl;
    return;
  }
  std::cout << "Virtual Serial Name (slave): " << virtualSerialName << std::endl;
  this->virtualPortName = std::string(virtualSerialName);
  close(slaveFd);
  this->setFileDescriptor(masterFd);
  this->setBaudrate(baud);
  this->setTimeout(timeout);
  this->setKeepAlive(keepAliveMs);
  if (this->setupAttributes() == false){
    std::cerr << "Gagal melakukan setup attribut virtual serial port" << std::endl;
  }
  else {
    std::cerr << "Virtual serial port ready" << std::endl;
  }
}

/**
 * @brief Destructor.
 *
 * Releases all allocated memory and closes the master port of the virtual serial port.
 */
VirtualSerial::~VirtualSerial(){
  /* destruksi dilakukan pada parrent object */
}

/**
 * @brief Sets the callback function for read and write operations on the master port of the virtual serial port.
 *
 * This method allows you to specify a callback function that will be used for handling data operations
 * (reading and writing) on the virtual serial port's master port.
 *
 * @param func Pointer to the callback function.
 * @param param Pointer to the parameter for the callback function.
 */
void VirtualSerial::setCallback(const void *func, void *param){
  this->callbackFunc = func;
  this->callbackParam = param;
}

/**
 * @brief Gets the callback function used for read and write operations on the master port of the virtual serial port.
 *
 * @return Pointer to the callback function.
 */
const void *VirtualSerial::getCallbackFunction(){
  return this->callbackFunc;
}

/**
 * @brief Gets the parameter pointer used for the callback function in read and write operations on the master port of the virtual serial port.
 *
 * @return Pointer to the parameter of the callback function.
 */
void *VirtualSerial::getCallbackParam(){
  return this->callbackParam;
}

/**
 * @brief Gets the name of the virtual serial port.
 *
 * @return The name of the virtual serial port (slave).
 */
std::string VirtualSerial::getVirtualPortName(){
  return this->virtualPortName;
}

/**
 * @brief Method to start executing the callback function.
 *
 * This method initiates the execution of the previously set callback function.
 *
 * @return `false` if the callback function has not been set up.
 * @return `true` if the callback function has been successfully executed.
 */
bool VirtualSerial::begin(){
  if (this->callbackFunc == nullptr) return false;
  void (*callback)(VirtualSerial &, void *) = (void (*)(VirtualSerial &, void *))this->callbackFunc;
  callback(*this, this->callbackParam);
  return true;
}