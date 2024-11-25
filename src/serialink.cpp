/*
 * $Id: serialink.cpp,v 1.0 2024/08/27 20:51:47 Jaya Wikrama Exp $
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
#include <string.h>
#include "serialink.hpp"

/**
 * @brief Default constructor.
 *
 * Initializes private data members and parameters to their default values.
 */
Serialink::Serialink(){
    this->usb = nullptr;
    this->isFormatValid = true;
    this->frameFormat = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * This constructor is used for specific purposes where the source device uses USB directly.
 *
 * @param usb The pointer of USB Serial Object.
 */
Serialink::Serialink(USBSerial *usb){
    this->usb = usb;
    this->isFormatValid = true;
    this->frameFormat = nullptr;
}

/**
 * @brief Destructor.
 *
 * Releases any allocated memory.
 */
Serialink::~Serialink(){
    if (this->frameFormat != nullptr){
        delete this->frameFormat;
        this->frameFormat = nullptr;
    }
}

/**
 * @brief Retrieves the memory address of the frame format.
 *
 * This function returns the address of the `frameFormat` data member.
 *
 * @return The memory address of the `frameFormat`.
 */
DataFrame *Serialink::getFormat(){
    return this->frameFormat;
}

/**
 * @brief Stops reading framed serial data.
 *
 * This function sets up variables that act as indicators for the validity of the received serial data,
 * allowing it to stop reading framed serial data from user space. It achieves this by using a post-execution
 * function configured within the `DataFrame` class.
 */
void Serialink::trigInvDataIndicator(){
    this->isFormatValid = false;
}

/**
 * @brief Performs serial data read operations with a custom frame format.
 *
 * This function executes serial data reading operations using a specific frame format.
 * The read serial data can be retrieved using the `__Serial::getBuffer__` method.
 *
 * @return 0 on success.
 * @return 1 if the port is not open.
 * @return 2 if a timeout occurs.
 * @return 3 if the frame format is not set up.
 * @return 4 if the frame data format is invalid.
 */
int Serialink::readFramedData(){
    if (this->frameFormat == nullptr) return 3;
    DataFrame *tmp = this->frameFormat;
    std::vector <unsigned char> vecUC;
    int ret = 0;
    void (*callback)(DataFrame &, void *) = nullptr;
    this->isFormatValid = true;
    while (tmp != nullptr){
        if (tmp->getExecuteFunction() != nullptr){
            callback = (void (*)(DataFrame &, void *))tmp->getExecuteFunction();
            callback(*tmp, tmp->getExecuteFunctionParam());
        }
        if (tmp->getType() == DataFrame::FRAME_TYPE_START_BYTES && tmp->getReference(vecUC) > 0){
            if (this->readStartBytes(vecUC.data(), vecUC.size())){
                ret = 2;
                break;
            }
        }
        else if (tmp->getType() == DataFrame::FRAME_TYPE_STOP_BYTES && tmp->getReference(vecUC) > 0){
            if (this->readStopBytes(vecUC.data(), vecUC.size())){
                ret = 2;
                break;
            }
        }
        else if (tmp->getType() == DataFrame::FRAME_TYPE_CONTENT_LENGTH ||
                 tmp->getType() == DataFrame::FRAME_TYPE_COMMAND ||
                 tmp->getType() == DataFrame::FRAME_TYPE_SN ||
                 tmp->getType() == DataFrame::FRAME_TYPE_RFU ||
                 tmp->getType() == DataFrame::FRAME_TYPE_BLOCK_NUMBER ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_1 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_2 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_3 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_4 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_5 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_6 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_7 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_8 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA_9 ||
                 tmp->getType() == DataFrame::FRAME_TYPE_VALIDATOR
        ){
            if (tmp->getSize() > 0){
                if (this->readNBytes(tmp->getSize()) == 0){
                    if (this->getBuffer(vecUC) > 0){
                        tmp->setData(vecUC);
                    }
                    else {
                        ret = 2;
                        break;
                    }
                }
            }
            else if (tmp->getNext() != nullptr) {
                if (tmp->getNext()->getType() == DataFrame::FRAME_TYPE_STOP_BYTES &&
                    tmp->getNext()->getReference(vecUC) > 0
                ){
                    if (this->readUntilStopBytes(vecUC.data(), vecUC.size()) == 0){
                        if (this->getBuffer(vecUC) > 0){
                            size_t sz = vecUC.size() - tmp->getNext()->getSize();
                            tmp->setData(vecUC.data(), sz);
                            if (tmp->getPostExecuteFunction() != nullptr){
                                callback = (void (*)(DataFrame &, void *))tmp->getPostExecuteFunction();
                                callback(*tmp, tmp->getPostExecuteFunctionParam());
                            }
                            tmp = tmp->getNext();
                            if (tmp->getExecuteFunction() != nullptr){
                                callback = (void (*)(DataFrame &, void *))tmp->getExecuteFunction();
                                callback(*tmp, tmp->getExecuteFunctionParam());
                            }
                        }
                    }
                    else {
                        ret = 2;
                        break;
                    }
                }
                else {
                    ret = 4;
                    break;
                }
            }
            else {
                ret = 4;
                break;
            }
        }
        else {
            ret = 4;
            break;
        }
        if (tmp->getPostExecuteFunction() != nullptr){
            callback = (void (*)(DataFrame &, void *))tmp->getPostExecuteFunction();
            callback(*tmp, tmp->getPostExecuteFunctionParam());
        }
        if (this->isFormatValid == false){
            ret = 4;
            break;
        }
        tmp = tmp->getNext();
        this->data.clear();
    }
    if (ret == 0){
        this->frameFormat->getAllData(this->data);
    }
    else if (ret != 4 && this->frameFormat != tmp && tmp->getType() == DataFrame::FRAME_TYPE_STOP_BYTES){
        DataFrame *fail = tmp;
        std::vector <unsigned char> dataFail;
        tmp = this->frameFormat;
        while (tmp != fail && tmp != nullptr){
            tmp->getData(vecUC);
            if (vecUC.size() > 0) dataFail.insert(dataFail.end(), vecUC.begin(), vecUC.end());
            tmp = tmp->getNext();
        }
        if (this->data.size() > 0) this->remainingData.insert(this->remainingData.begin(), this->data.begin(), this->data.end());
        this->data.clear();
        if (!dataFail.empty()){
            if (dataFail.size() > 1) this->remainingData.insert(this->remainingData.begin(), dataFail.begin() + 1, dataFail.end());
            if (dataFail.size() > 1) this->data.insert(this->data.begin(), dataFail.begin(), dataFail.begin() + 1);
        }
    }
    else if (tmp != nullptr){
        DataFrame *fail = tmp;
        std::vector <unsigned char> dataFail;
        tmp = this->frameFormat;
        while (tmp != fail && tmp != nullptr){
            tmp->getData(vecUC);
            if (vecUC.size() > 0) dataFail.insert(dataFail.end(), vecUC.begin(), vecUC.end());
            tmp = tmp->getNext();
        }
        if (dataFail.size() > 0) this->data.insert(this->data.begin(), dataFail.begin(), dataFail.end());
    }
    return ret;
}

/**
 * @brief Performs serial data write operations with a custom frame format.
 *
 * This function executes serial data write operations using a specific frame format.
 *
 * @return 0 on success.
 * @return 1 if the port is not open.
 * @return 2 if a timeout occurs.
 * @return 3 if there is no data to write.
 */
int Serialink::writeFramedData(){
    std::vector <unsigned char> buffer;
    if (this->frameFormat->getAllData(buffer) > 0){
        return this->writeData(buffer);
    }
    return 3;
}

/**
 * @brief Retrieves a buffer of data read and stored in the Framed Data within a specified range.
 *
 * This method extracts data from the buffer that has been successfully read and stored in the Framed Data
 * within the specified range. This version is suitable for data with unique Frame Formats in each frame
 * (no duplicate Frame Types).
 *
 * @param begin Reference to the starting point for data extraction.
 * @param end Reference to the ending point for data extraction.
 * @return A vector containing the data.
 */
std::vector <unsigned char> Serialink::getSpecificBufferAsVector(DataFrame::FRAME_TYPE_t begin, DataFrame::FRAME_TYPE_t end){
    DataFrame *tmpBegin = (*this)[begin];
    DataFrame *tmpEnd = (*this)[end];
    return this->frameFormat->getSpecificDataAsVector(tmpBegin, tmpEnd);
}

/**
 * @brief Overloaded method of __getSpecificBufferAsVector__.
 *
 * This method performs the same data extraction operation as the other overload but is designed to handle
 * cases where duplicate Frame Formats exist within the Framed Data. It retrieves data from the buffer
 * that has been successfully read and stored within the specified range.
 *
 * @param begin Pointer to the starting point for data extraction.
 * @param end Pointer to the ending point for data extraction.
 * @return A vector containing the data.
 */
std::vector <unsigned char> Serialink::getSpecificBufferAsVector(const DataFrame *begin, const DataFrame *end){
    return this->frameFormat->getSpecificDataAsVector(begin, end);
}

Serialink& Serialink::operator=(const DataFrame &obj){
    if (this->frameFormat != nullptr){
        delete this->frameFormat;
        this->frameFormat = nullptr;
    }
    DataFrame &ncObj = const_cast<DataFrame&>(obj);
    std::vector <unsigned char> ref;
    ncObj.getReference(ref);
    this->frameFormat = new DataFrame(
     static_cast<DataFrame::FRAME_TYPE_t>(ncObj.getType()),
      ncObj.getSize(),
      ref.data(),
      ncObj.getExecuteFunction(),
      ncObj.getExecuteFunctionParam(),
      ncObj.getPostExecuteFunction(),
      ncObj.getPostExecuteFunctionParam()
    );
    if (this->frameFormat != nullptr){
        if (ncObj.getNext() != nullptr)
            *(this->frameFormat) += *(ncObj.getNext());
    }
    return *this;
}

Serialink& Serialink::operator+=(const DataFrame &obj){
    *(this->frameFormat) += obj;
    return *this;
}

Serialink& Serialink::operator+(const DataFrame &obj){
    *this += obj;
    return *this;
}

DataFrame* Serialink::operator[](int idx){
    int i = 0;
    DataFrame *tmp = this->frameFormat;
    if (tmp == nullptr || i < 0) return nullptr;
    while(tmp != nullptr){
        if(i == idx) return tmp;
        i++;
        tmp = tmp->getNext();
    }
    return nullptr;
}

DataFrame* Serialink::operator[](DataFrame::FRAME_TYPE_t type){
    DataFrame *tmp = this->frameFormat;
    if (tmp == nullptr) return nullptr;
    while(tmp != nullptr){
        if(tmp->getType() == type) return tmp;
        tmp = tmp->getNext();
    }
    return nullptr;
}

DataFrame* Serialink::operator[](std::pair <DataFrame::FRAME_TYPE_t, int> params){
    int i = 0;
    DataFrame *tmp = this->frameFormat;
    if (tmp == nullptr || i < 0) return nullptr;
    while(tmp != nullptr){
        if (tmp->getType() == params.first){
            if(i == params.second) return tmp;
            i++;
        }
        tmp = tmp->getNext();
    }
    return nullptr;
}

