#ifndef __VIRTUAL_SERIAL_DEVICE_HPP__
#define __VIRTUAL_SERIAL_DEVICE_HPP__

#include "serial.hpp"

class VirtualSerial : public Serial {
  private:
    std::string virtualPortName;
    const void *callbackFunc;
    void *callbackParam;
  public:
    /**
     * @brief Default constructor.
     *
     * Berfungsi untuk melakukan menciptakan virtual serial port. Nama atau alamat virtual port untuk slave dapat diambil dengan method __getVirtualPortName__.
     * baud = B9600
     * timeout = 10 (1 detik)
     * keepAliveMs = 0
     * Initialize mutex
     */
    VirtualSerial();

    /**
     * @brief custom constructor.
     *
     * Berfungsi untuk melakukan menciptakan virtual serial port. Nama atau alamat virtual port untuk slave dapat diambil dengan method __getVirtualPortName__.
     * @param port port device serial.
     * @param baud baudrate.
     * @param timeout timeout per 100ms.
     * @param keepAliveMs waktu dalam Milliseconds.
     */
    VirtualSerial(speed_t baud, unsigned int timeout, unsigned int keepAliveMs);

    /**
     * @brief Destructor.
     *
     * Berfungsi untuk melakukan release setiap memory yang dialokasikan dan menutup port master dari virtual serial.
     */
    ~VirtualSerial();

    /**
     * @brief setter untuk callback function yang dapat digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
     *
     * @param func pointer dari callback function.
     * @param param parameter pointer dari callback function.
     */
    void setCallback(const void *func, void *param);

    /**
     * @brief getter untuk callback function yang digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
     *
     * @return pointer dari callback function.
     */
    const void *getCallbackFunction();

    /**
     * @brief getter untuk callback function yang digunakan dalam pengoperasian baca dan tulis data port master dari virtual serial port.
     *
     * @return parameter pointer dari callback function.
     */
    void *getCallbackParam();

    /**
     * @brief getter untuk nama virtual serial port.
     *
     * @return nama virtual serial port (slave).
     */
    std::string getVirtualPortName();

    /**
     * @brief Method untuk memulai menjalankan callback function.
     *
     * @return false jika callback function belum di-setup.
     * @return true jika callback function telah selesai dijalankan.
     */
    bool begin();
};

#endif