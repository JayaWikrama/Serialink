/*
 * $Id: data-formating.hpp,v 1.0.0 2024/12/09 19:01:34 Jaya Wikrama Exp $
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
#include <iomanip>
#include <stdexcept>
#include "validator.hpp"
#include "data-formating.hpp"

/**
 * @brief Constructor of ProtocolFormat object.
 *
 * This constructor initializes the Protocol or Data Frame Format of <your-module-or-device>.
 * If there is a failure while initializing the object, the method will throw a
 * runtime_error signal which you can retrieve in the try exception mechanism.
 *
 * @param obj The main object of serial handler.
 */
ProtocolFormat::ProtocolFormat(Serialink &obj){
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
   * example: 3132333435363738159039302D3D
   */
  /* Configure Frame Format */
  this->frameProtocol = new DataFrame(DataFrame::FRAME_TYPE_START_BYTES, "1234");
  if (this->frameProtocol == nullptr) throw std::runtime_error(std::string(__func__) + ": failed to allocate memory!");
  DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
  /* Setup the handler function to determine the data length of DataFrame::FRAME_TYPE_DATA.
   * This function is called after all data from DataFrame::FRAME_TYPE_COMMAND is received.
   */
  cmdBytes.setPostExecuteFunction((const void *) &(ProtocolFormat::setDataLength), (void *) &obj);
  DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
  DataFrame crcValidatorBytes(DataFrame::FRAME_TYPE_VALIDATOR, 2);
  /* Setup the handler function to validate Data by using crc16 validation.
   * This function is called after all data from DataFrame::FRAME_TYPE_VALIDATOR is received.
   */
  crcValidatorBytes.setPostExecuteFunction((const void *) &(ProtocolFormat::validate), (void *) &obj);
  DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
  /* Setup Frame Format to Serialink com */
  *(this->frameProtocol) += cmdBytes + dataBytes + crcValidatorBytes + stopBytes;
  obj = *(this->frameProtocol);
}

/**
 * @brief Destructor.
 *
 * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
 * It ensures that all allocated resources are properly freed, preventing memory leaks.
 */
ProtocolFormat::~ProtocolFormat(){
  if (this->frameProtocol != nullptr) delete this->frameProtocol;
}

/**
 * @brief Sets the length of the main data on protocol.
 *
 * This setter function configures the length of main data part of protocol or data frame format.
 * Setup this method as post execution function on specific frame (part of protocol or data frame format).
 * If there is a failure while get the target frame from main object, the method will throw a
 * runtime_error signal which you can retrieve in the try exception mechanism.
 *
 * @param frame frames that are currently being processed.
 * @param ref reference pointer (pointer of main object).
 */
void ProtocolFormat::setDataLength(DataFrame &frame, void *ref){
  int data = 0;
  /* Get Serialink object form function param */
  Serialink *tmp = (Serialink *) ref;
  /* Get DataFrame target */
  DataFrame *target = (*tmp)[DataFrame::FRAME_TYPE_DATA];
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
    tmp->trigInvDataIndicator();
  }
}

/**
 * @brief Data validator.
 *
 * This method functions to validate data using the validator method specified by the protocol.
 * Setup this method as post execution function on specific frame (part of protocol or data frame format).
 * If there is a failure while get the specific data, the method will throw a runtime_error signal which
 * you can retrieve in the try exception mechanism.
 *
 * @param frame frames that are currently being processed.
 * @param ref reference pointer (pointer of main object).
 */
void ProtocolFormat::validate(DataFrame &frame, void *ref){
  /* Get Serialink object form function param */
  Serialink *obj = (Serialink *) ref;
  /* prepare validator */
  Validator validator(Validator::VALIDATOR_TYPE_CRC16);
  unsigned short init = 0x0000;
  unsigned short poly = 0x1021;
  validator.setInitialValue((const unsigned char *) &init, 2);
  validator.setPoly((const unsigned char *) &poly, 2);
  /* Validate Format */
  if (validator.validate(frame.getDataAsVector().data(), obj->getFormat(), DataFrame::FRAME_TYPE_START_BYTES, DataFrame::FRAME_TYPE_DATA) == false){
    /* invalid crc -> trigger stop flag to __readFramedData__ method */
    obj->trigInvDataIndicator();
    std::cout << __func__ << ": data invalid!" << std::endl;
    std::cout << __func__ << ": received checksum: ";
    ProtocolFormat::displayData(frame.getDataAsVector());
    std::cout << __func__ << ": calc. checksum   : ";
    ProtocolFormat::displayData(validator.getChecksum(obj->getFormat(), DataFrame::FRAME_TYPE_START_BYTES, DataFrame::FRAME_TYPE_DATA));
  }
}

/**
 * @brief Framed data builder.
 *
 * This method functions is responsible to build framed data that ready to be send/write to serial devices. 
 *
 * @param data The main data of protocol.
 */
std::vector <unsigned char> ProtocolFormat::buildCommand(const unsigned char *data, size_t sz){
  unsigned char cmd = 0x36;
  if (sz == 3) cmd = 0x35;
  /* Initialize Frame */
  DataFrame *tmp = nullptr;
  DataFrame reqFrame(DataFrame::FRAME_TYPE_START_BYTES, this->frameProtocol->getDataAsVector());
  reqFrame += *(this->frameProtocol->getNext());
  /* Set data length */
  tmp = reqFrame[DataFrame::FRAME_TYPE_COMMAND];
  if (tmp == nullptr) throw std::runtime_error(std::string(__func__) + ": failed to access command frame!");
  tmp->setData(&cmd, 1);
  /* Set data */
  tmp = reqFrame[DataFrame::FRAME_TYPE_DATA];
  if (tmp == nullptr) throw std::runtime_error(std::string(__func__) + ": failed to access data frame!");
  tmp->setData(data, sz);
  /* Prepare validator */
  Validator validator(Validator::VALIDATOR_TYPE_CRC16);
  unsigned short init = 0x0000;
  unsigned short poly = 0x1021;
  validator.setInitialValue((const unsigned char *) &init, 2);
  validator.setPoly((const unsigned char *) &poly, 2);
  /* Get Checksum */
  std::vector <unsigned char> checksum = validator.getChecksum(reqFrame, DataFrame::FRAME_TYPE_START_BYTES, DataFrame::FRAME_TYPE_DATA);
  /* Sets Checksum */
  tmp = reqFrame[DataFrame::FRAME_TYPE_VALIDATOR];
  if (tmp == nullptr) throw std::runtime_error(std::string(__func__) + ": failed to access checksum frame!");
  tmp->setData(checksum);
  std::vector <unsigned char> result = reqFrame.getAllDataAsVector();
  return result;
}

/**
 * @brief Overloading method of framed data builder.
 *
 * This method functions is responsible to build framed data that ready to be send/write to serial devices. 
 *
 * @param data The main data of protocol.
 */
std::vector <unsigned char> ProtocolFormat::buildCommand(const std::vector <unsigned char> &data){
  return this->buildCommand(data.data(), data.size());
}

/**
 * @brief Display Hex data.
 *
 * This method functions is responsible to display hexadecimal data. 
 */
void ProtocolFormat::displayData(const unsigned char *data, size_t sz){
  for (int i = 0; i < sz; i++) {
    std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)(data[i]);
    std::cout << " ";
  }
  std::cout << std::endl;
}

/**
 * @brief Overloading method of Hex data display.
 *
 * This method functions is responsible to display hexadecimal data. 
 */
void ProtocolFormat::displayData(const std::vector <unsigned char> &data){
  for (auto i = data.begin(); i < data.end(); ++i) {
    std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)*i;
    std::cout << " ";
  }
  std::cout << std::endl;
}