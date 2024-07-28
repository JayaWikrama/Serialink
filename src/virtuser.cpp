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
 * Berfungsi untuk melakukan menciptakan virtual serial port. Nama atau alamat virtual port untuk slave dapat diambil dengan method __getVirtualPortName__.
 * baud = B9600
 * timeout = 10 (1 detik)
 * keepAliveMs = 0
 * Initialize mutex
 */
VirtualSerial::VirtualSerial(){
  int masterFd = -1;
  int slaveFd = -1;
  char virtualSerialName[128];
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
 * @brief custom constructor.
 *
 * Berfungsi untuk melakukan menciptakan virtual serial port. Nama atau alamat virtual port untuk slave dapat diambil dengan method __getVirtualPortName__.
 * @param port port device serial.
 * @param baud baudrate.
 * @param timeout timeout per 100ms.
 * @param keepAliveMs waktu dalam Milliseconds.
 */
VirtualSerial::VirtualSerial(speed_t baud, unsigned int timeout, unsigned int keepAliveMs){
  int masterFd = -1;
  int slaveFd = -1;
  char virtualSerialName[128];
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
 * Berfungsi untuk melakukan release setiap memory yang dialokasikan dan menutup port master dari virtual serial.
 */
VirtualSerial::~VirtualSerial(){
  /* destruksi dilakukan pada parrent object */
}

/**
 * @brief setter untuk callback function yang dapat digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
 *
 * @param func pointer dari callback function.
 * @param param parameter pointer dari callback function.
 */
void VirtualSerial::setCallback(const void *func, void *param){
  this->callbackFunc = func;
  this->callbackParam = param;
}

/**
 * @brief getter untuk callback function yang digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
 *
 * @return pointer dari callback function.
 */
const void *VirtualSerial::getCallbackFunction(){
  return this->callbackFunc;
}

/**
 * @brief getter untuk callback function yang digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
 *
 * @return parameter pointer dari callback function.
 */
void *VirtualSerial::getCallbackParam(){
  return this->callbackParam;
}

/**
 * @brief getter untuk nama virtual serial port.
 *
 * @return nama virtual serial port (slave).
 */
std::string VirtualSerial::getVirtualPortName(){
  return this->virtualPortName;
}

/**
 * @brief Method untuk memulai menjalankan callback function.
 *
 * @return false jika callback function belum di-setup.
 * @return true jika callback function telah selesai dijalankan.
 */
bool VirtualSerial::begin(){
  if (this->callbackFunc == nullptr) return false;
  void (*callback)(VirtualSerial &, void *) = (void (*)(VirtualSerial &, void *))this->callbackFunc;
  callback(*this, this->callbackParam);
  return true;
}