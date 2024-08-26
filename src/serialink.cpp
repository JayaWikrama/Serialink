#include <iostream>
#include <string.h>
#include "serialink.hpp"

/**
 * @brief Default constructor.
 *
 * Berfungsi untuk melakukan setup private data dan parameter ke nilai default.
 */
Serialink::Serialink(){
    this->isFormatValid = true;
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
 * @brief berfungsi untuk menghentikan pembacaan framed data serial.
 *
 * Berfungsi untuk melakukan setup variable yang menjadi indikator valid tidaknya data serial yang diterima sehingga dapat menghentikan pembacaan framed data serial dari user space melalui post execution function yang di setup pada class DataFrame.
 */
void Serialink::trigInvDataIndicator(){
    this->isFormatValid = false;
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika frame format belum di setup.
 * @return 4 jika terdapat format frame data yang tidak valid.
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
                 tmp->getType() == DataFrame::FRAME_TYPE_DATA ||
                 tmp->getType() == DataFrame::FRAME_TYPE_VALIDATOR ||
                 tmp->getType() == DataFrame::FRAME_TYPE_COMMAND
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
        if (dataFail.size() > 0) this->remainingData.insert(this->remainingData.begin(), dataFail.begin() + 1, dataFail.end());
        this->data.clear();
        this->data.insert(this->data.begin(), dataFail.begin(), dataFail.begin() + 1);
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
 * @brief berfungsi untuk melakukan operasi penulisan data serial dengan format frame khusus.
 *
 * Berfungsi untuk melakukan operasi penulisan data serial dengan format frame khusus.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika tidak ada data yang akan ditulis.
 */
int Serialink::writeFramedData(){
    std::vector <unsigned char> buffer;
    if (this->frameFormat->getAllData(buffer) > 0){
        return this->writeData(buffer);
    }
    return 3;
}

/**
 * @brief berfungsi untuk mengambil data buffer yang berhasil terbaca dan tersimpan di dalam Framed Data dengan range tertentu.
 *
 * Berfungsi untuk melakukan operasi pengambilan data buffer yang berhasil terbaca dan tersimpan di dalam Framed Data dengan range tertentu. Method dengan parameter ini cocok digunakan pada data dengan Frame Format yang unik setiap Frame-nya (tidak ada type Frame yang kembar).
 * @param begin merupakan referensi titik awal pengambilan data.
 * @param end merupakan referensi titik akhir pengambilan data.
 * @return data dalam bentuk vector.
 */
std::vector <unsigned char> Serialink::getSpecificBufferAsVector(DataFrame::FRAME_TYPE_t begin, DataFrame::FRAME_TYPE_t end){
    DataFrame *tmpBegin = (*this)[begin];
    DataFrame *tmpEnd = (*this)[end];
    return this->frameFormat->getSpecificDataAsVector(tmpBegin, tmpEnd);
}

/**
 * @brief overloading dari method __getSpecificBufferAsVector__.
 *
 * Berfungsi untuk melakukan operasi pengambilan data buffer yang berhasil terbaca dan tersimpan di dalam Framed Data dengan range tertentu. Method dengan parameter ini cocok digunakan jika terdapat Frame Format yang kembar pada Framed Data yang dibangun.
 * @param begin merupakan referensi titik awal pengambilan data.
 * @param end merupakan referensi titik akhir pengambilan data.
 * @return data dalam bentuk vector.
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

