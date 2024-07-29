#include <iostream>
#include <string.h>
#include "serialink.hpp"

/**
 * @brief Default constructor.
 *
 * Berfungsi untuk melakukan setup private data dan parameter ke nilai default.
 */
Serialink::Serialink(){
    this->frameFormat = nullptr;
}

/**
 * @brief Destructor.
 *
 * Berfungsi untuk melakukan release setiap memory yang dialokasikan.
 */
Serialink::~Serialink(){
    if (this->frameFormat != nullptr){
        delete this->frameFormat;
        this->frameFormat = nullptr;
    }
}

/**
 * @brief berfungsi untuk mengambil alamat memory frameFormat.
 *
 * Berfungsi untuk mengambil informasi alamat memory frameFormat.
 * @return alamat memory frameFormat.
 */
DataFrame *Serialink::getFormat(){
    return this->frameFormat;
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika frame format belum di setup.
 */
int Serialink::readFramedData(){
    if (this->frameFormat == nullptr) return 3;
    DataFrame *tmp = this->frameFormat;
    std::vector <unsigned char> vecUC;
    int ret = 2;
    void (*callback)(DataFrame &, void *) = nullptr;
    while (tmp != nullptr){
        if (tmp->getExecuteFunction() != nullptr){
            callback = (void (*)(DataFrame &, void *))tmp->getExecuteFunction();
            callback(*tmp, tmp->getExecuteFunctionParam());
        }
        if (tmp->getType() == DataFrame::FRAME_TYPE_START_BYTES && tmp->getReference(vecUC) > 0){
            if (this->readStartBytes(vecUC.data(), vecUC.size())){
                break;
            }
        }
        else if (tmp->getType() == DataFrame::FRAME_TYPE_STOP_BYTES && tmp->getReference(vecUC) > 0){
            if (this->readStartBytes(vecUC.data(), vecUC.size())){
                break;
            }
        }
        else if ((tmp->getType() == DataFrame::FRAME_TYPE_CONTENT_LENGTH ||
                  tmp->getType() == DataFrame::FRAME_TYPE_DATA ||
                  tmp->getType() == DataFrame::FRAME_TYPE_VALIDATOR ||
                  tmp->getType() == DataFrame::FRAME_TYPE_COMMAND
                 ) &&
                 tmp->getSize() > 0
        ){
            if (this->readNBytes(tmp->getSize()) == 0){
                if (this->getBuffer(vecUC) > 0){
                    tmp->setData(vecUC);
                }
            }
        }
        if (tmp->getPostExecuteFunction() != nullptr){
            callback = (void (*)(DataFrame &, void *))tmp->getPostExecuteFunction();
            callback(*tmp, tmp->getPostExecuteFunctionParam());
        }
        tmp = tmp->getNext();
    }
    return 0;
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