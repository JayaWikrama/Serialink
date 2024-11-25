/*
 * $Id: serial.cpp,v 1.0 2024/08/25 13:03:53 Jaya Wikrama Exp $
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
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <sys/time.h>
#include "serial.hpp"

/**
 * @brief Sets the file descriptor.
 *
 * This setter function assigns a value to the file descriptor.
 *
 * @param fd The file descriptor to be set.
 */
#if defined(PLATFORM_POSIX) || defined(__linux__)
void Serial::setFileDescriptor(int fd)
#else
void Serial::setFileDescriptor(HANDLE fd)
#endif
{
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    this->fd = fd;
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
}

/**
 * @brief Gets the file descriptor.
 *
 * This getter function retrieves the current value of the file descriptor.
 *
 * @return The current file descriptor.
 */
#if defined(PLATFORM_POSIX) || defined(__linux__)
    int Serial::getFileDescriptor()
#else
    HANDLE Serial::getFileDescriptor()
#endif
{
    return this->fd;
}

/**
 * @brief Configures serial port attributes.
 *
 * This function sets up or configures the attributes of the file descriptor for a successfully opened serial port.
 *
 * @return true if the configuration is successful.
 * @return false if the configuration fails.
 */
bool Serial::setupAttributes(){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    bool result = false;
#if defined(PLATFORM_POSIX) || defined(__linux__)
    struct termios ttyAttr;
    memset (&ttyAttr, 0, sizeof(ttyAttr));
    result = (tcgetattr(this->fd, &ttyAttr) == 0);
#else
    result = FlushFileBuffers(this->fd);
#endif
    if (result == false){
        pthread_mutex_unlock(&(this->mtx));
        pthread_mutex_unlock(&(this->wmtx));
        return false;
    }
#if defined(PLATFORM_POSIX) || defined(__linux__)
    cfsetospeed (&ttyAttr, this->baud);
    ttyAttr.c_cflag = (ttyAttr.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    ttyAttr.c_iflag &= ~IGNBRK; // disable break processing
    ttyAttr.c_lflag = 0; // no signaling chars, no echo, no canonical processing
    ttyAttr.c_oflag = 0; // no remapping, no delays
    ttyAttr.c_cc[VMIN]  = 0; // blocking mode
    ttyAttr.c_cc[VTIME] = this->timeout; // per 100ms read timeout
    ttyAttr.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    ttyAttr.c_cflag |= (CLOCAL | CREAD); // ignore modem controls, enable reading
    ttyAttr.c_cflag &= ~(PARENB | PARODD); // shut off parity
    ttyAttr.c_cflag |= 0;
    ttyAttr.c_cflag &= ~CSTOPB;
    ttyAttr.c_cflag &= ~CRTSCTS;
    ttyAttr.c_iflag &= ~(INLCR | ICRNL);
    result = (tcsetattr (this->fd, TCSANOW, &ttyAttr) == 0);
#else
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = this->timeout;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = this->timeout;
    timeouts.WriteTotalTimeoutMultiplier = 50;
    result = SetCommTimeouts(this->fd, &timeouts);
    if (result == true){
        DCB state = {0};
        state.DCBlength=sizeof(DCB);
        result = GetCommState(this->fd, &state);
        if (result == true){
            state.BaudRate = static_cast<uint32_t>(this->baud);
            state.ByteSize = 8;
            state.Parity = NOPARITY;
            state.StopBits = ONESTOPBIT;
            result = SetCommState(this->fd, &state);
        }
    }
#endif
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
    return result;
}

/**
 * @brief Default constructor.
 *
 * This constructor initializes private data members and parameters to their default values, including:
 * - `fd = -1` : File descriptor is set to an invalid state.
 * - `baud = B9600` : Baud rate is set to 9600 bps.
 * - `timeout = 10` : Timeout is set to 10 deciseconds (1 second).
 * - `keepAliveMs = 0` : Keep-alive interval is set to 0 milliseconds.
 * - `port = "/dev/ttyUSB0"` : Default serial port is set to `/dev/ttyUSB0`.
 * - Initializes the mutex for thread safety.
 */
Serial::Serial(){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = B9600;
    this->timeout = 10;
    this->keepAliveMs = 0;
    this->port = "/dev/ttyUSB0";
    pthread_mutex_init(&(this->mtx), NULL);
    pthread_mutex_init(&(this->wmtx), NULL);
    this->usb = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * This constructor initializes private data members to their default values (except for `port`, `baud`, and `timeout`), including:
 * - `fd = -1` : File descriptor is set to an invalid state.
 * - `keepAliveMs = 0` : Keep-alive interval is set to 0 milliseconds.
 * - Initializes the mutex for thread safety.
 *
 * @param port The serial port device (e.g., "/dev/ttyUSB0").
 * @param baud The baud rate for the serial communication.
 * @param timeout Timeout value in units of 100 milliseconds (e.g., `10` equals 1 second).
 */
Serial::Serial(const std::string port, speed_t baud, unsigned int timeout){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = baud;
    this->timeout = timeout;
    this->keepAliveMs = 0;
    this->port = port;
    pthread_mutex_init(&(this->mtx), NULL);
    pthread_mutex_init(&(this->wmtx), NULL);
    this->usb = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * This constructor initializes private data members to their default values (except for `port`, `baud`, `timeout`, and `keepAliveMs`), including:
 * - `fd = -1` : File descriptor is set to an invalid state.
 * - Initializes the mutex for thread safety.
 *
 * @param port The serial port device (e.g., "/dev/ttyUSB0").
 * @param baud The baud rate for the serial communication.
 * @param timeout Timeout value in units of 100 milliseconds (e.g., `10` equals 1 second).
 * @param keepAliveMs Keep-alive interval in milliseconds.
 */
Serial::Serial(const std::string port, speed_t baud, unsigned int timeout, unsigned int keepAliveMs){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = baud;
    this->timeout = timeout;
    this->keepAliveMs = keepAliveMs;
    this->port = port;
    pthread_mutex_init(&(this->mtx), NULL);
    pthread_mutex_init(&(this->wmtx), NULL);
    this->usb = nullptr;
}

/**
 * @brief Custom constructor.
 *
 * This constructor is used for specific purposes where the source device uses USB directly.
 *
 * @param usb The pointer of USB Serial Object.
 */
Serial::Serial(USBSerial *usb){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = B9600;
    this->timeout = 10;
    this->keepAliveMs = 0;
    this->port = "/dev/ttyUSB0";
    pthread_mutex_init(&(this->mtx), NULL);
    pthread_mutex_init(&(this->wmtx), NULL);
    this->usb = usb;
}

/**
 * @brief Destructor.
 *
 * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
 * It ensures that all allocated resources are properly freed, preventing memory leaks.
 */
Serial::~Serial(){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd > 0){
        close(this->fd);
        this->fd = -1;
    }
#else
    CloseHandle(this->fd);
#endif
    if (this->usb != nullptr) delete (this->usb);
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
    pthread_mutex_destroy(&(this->mtx));
    pthread_mutex_destroy(&(this->wmtx));
}

/**
 * @brief Sets the serial port device.
 *
 * This setter function configures the serial port device to be used for communication.
 *
 * @param port The serial port device (e.g., "/dev/ttyUSB0").
 */
void Serial::setPort(const std::string port){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    this->port = port;
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
}

/**
 * @brief Sets the baud rate for communication.
 *
 * This setter function configures the baud rate used for serial communication.
 *
 * @param baud The baud rate (e.g., `B9600` for 9600 bps).
 */
void Serial::setBaudrate(speed_t baud){
    pthread_mutex_lock(&(this->mtx));
    this->baud = baud;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief Sets the communication timeout.
 *
 * This setter function configures the timeout for serial communication. The timeout value is specified in units of 100 milliseconds.
 *
 * @param timeout The timeout value (e.g., `10` for a 1-second timeout).
 */
void Serial::setTimeout(unsigned int timeout){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    this->timeout = timeout;
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
}

/**
 * @brief Sets the keep-alive interval for communication.
 *
 * This setter function configures the maximum wait time for receiving the next byte of serial data after the initial byte has been received. This helps maintain the connection by ensuring timely data reception.
 *
 * @param keepAliveMs The keep-alive interval in milliseconds.
 */
void Serial::setKeepAlive(unsigned int keepAliveMs){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    this->keepAliveMs = keepAliveMs;
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
}

/**
 * @brief Gets the serial port.
 *
 * This getter function retrieves the serial port currently being used.
 *
 * @return The serial port as a string (e.g., "/dev/ttyUSB0").
 */
std::string Serial::getPort(){
    return this->port;
}

/**
 * @brief Gets the baud rate for serial communication.
 *
 * This getter function retrieves the baud rate currently configured for serial communication.
 *
 * @return The baud rate (e.g., `B9600` for 9600 bps).
 */
speed_t Serial::getBaudrate(){
    return this->baud;
}

/**
 * @brief Gets the communication timeout.
 *
 * This getter function retrieves the timeout value configured for serial communication. The timeout is specified in units of 100 milliseconds.
 *
 * @return The timeout value (e.g., `10` for a 1-second timeout).
 */
unsigned int Serial::getTimeout(){
    return this->timeout;
}

/**
 * @brief Gets the keep-alive interval for communication.
 *
 * This getter function retrieves the maximum wait time configured for receiving the next byte of serial data after the initial byte has been successfully received. The interval is specified in milliseconds.
 *
 * @return The keep-alive interval in milliseconds.
 */
unsigned int Serial::getKeepAlive(){
    return this->keepAliveMs;
}

/**
 * @brief Opens the serial port for communication.
 *
 * This function attempts to open the specified serial port for communication. It configures the port according to the current settings and prepares it for data transfer.
 *
 * @return 0 if the port is successfully opened.
 * @return 1 if the port fails to open.
 */
int Serial::openPort(){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    if (this->usb != nullptr){
        int result = this->usb->openDevice();
        pthread_mutex_unlock(&(this->wmtx));
        pthread_mutex_unlock(&(this->mtx));
        return result;
    }
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = open (this->port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (this->fd <= 0)
#else
    this->fd = CreateFileA(this->port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (this->fd == INVALID_HANDLE_VALUE)
#endif
    {
        pthread_mutex_unlock(&(this->mtx));
        pthread_mutex_unlock(&(this->wmtx));
		return 1;
	}
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
    bool success = this->setupAttributes();
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
    if (success == false){
#if defined(PLATFORM_POSIX) || defined(__linux__)
        close(this->fd);
        this->fd = -1;
#else
        CloseHandle(this->fd);
#endif
        pthread_mutex_unlock(&(this->mtx));
        pthread_mutex_unlock(&(this->wmtx));
        return 1;
    }
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
    return 0;
}

#if defined(PLATFORM_POSIX) || defined(__linux__)
/**
 * @brief Checks for available input bytes.
 *
 * This function checks whether there are any bytes available in the serial buffer for reading.
 *
 * @return `true` if there are bytes available in the serial buffer.
 * @return `false` if there are no bytes available in the serial buffer.
 */
bool Serial::isInputBytesAvailable(){
    pthread_mutex_lock(&(this->mtx));
    if (this->usb != nullptr){
        pthread_mutex_unlock(&(this->mtx));
        return true;
    }
    long inputBytes = 0;
    if (ioctl(this->fd, FIONREAD, &inputBytes) != 0){
        pthread_mutex_unlock(&(this->mtx));
        return false;
    }
    pthread_mutex_unlock(&(this->mtx));
    return (inputBytes > 0 ? true : false);
}
#endif

/**
 * @brief Performs a serial data read operation.
 *
 * This function reads data from the serial port without separating the successfully read data into the desired size and remaining data. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param sz The number of bytes to read. A value of `0` means that the read operation is unlimited (up to the `keepAliveMs` timeout).
 * @param dontSplitRemainingData A flag to disable automatic data splitting based on the amount of data requested.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readData(size_t sz, bool dontSplitRemainingData){
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd <= 0 && this->usb == nullptr){
        pthread_mutex_unlock(&(this->mtx));
        return 1;
    }
    ssize_t bytes = 0;
#else
    long unsigned int bytes = 0;
#endif
    int idx = 0;
    unsigned char tmp[1024];
    this->data.clear();
    if (this->remainingData.size() > 0){
        this->data.assign(this->remainingData.begin(), this->remainingData.end());
        this->remainingData.clear();
    }
    do {
#if defined(PLATFORM_POSIX) || defined(__linux__)
        if (this->data.size() > 0) {
            if (this->keepAliveMs == 0) break;
            pthread_mutex_unlock(&(this->mtx));
            static struct timeval tvStart;
            static struct timeval tvEnd;
            static long diffTime = 0;
            gettimeofday(&tvStart, NULL);
            do {
                if (this->isInputBytesAvailable() == true){
                    break;
                }
                else {
                    usleep(1000);
                }
                gettimeofday(&tvEnd, NULL);
                diffTime = static_cast<long>((tvEnd.tv_sec - tvStart.tv_sec) * 1000) + static_cast<long>((tvEnd.tv_usec - tvStart.tv_usec) / 1000);
            } while (diffTime < static_cast<long>(this->keepAliveMs));
            if (this->isInputBytesAvailable() == false){
                pthread_mutex_lock(&(this->mtx));
                break;
            }
            pthread_mutex_lock(&(this->mtx));
        }
        if (this->usb == nullptr){
            bytes = read(this->fd, (void *) tmp, sizeof(tmp));
        }
        else {
            bytes = this->usb->readDevice(tmp, sizeof(tmp));
        }
#else
        bool success = ReadFile(this->fd, tmp, sizeof(tmp), &bytes, NULL);
        if (success == false){
            bytes = 0;
        }
#endif
        if (bytes > 0){
            for (idx = 0; idx < bytes; idx++){
                this->data.push_back(tmp[idx]);
            }
        }
    } while (bytes > 0 && (sz == 0 || this->data.size() < sz));
    if (this->data.size() == 0){
        pthread_mutex_unlock(&(this->mtx));
        return 2;
    }
    if (dontSplitRemainingData == false && sz > 0 && this->data.size() > sz){
        this->remainingData.assign(this->data.begin() + sz, this->data.end());
        this->data.erase(this->data.begin() + sz, this->data.end());
    }
    pthread_mutex_unlock(&(this->mtx));
    return 0;
}

/**
 * @brief Overloaded method for `readData` to perform serial data reading.
 *
 * This overloaded method performs a serial data read operation. The successfully read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param sz The number of bytes to read. A value of `0` means that the read operation is unlimited (up to the `keepAliveMs` timeout).
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readData(size_t sz){
    return this->readData(sz, false);
}

/**
 * @brief Overloaded method for `readData` to perform serial data reading.
 *
 * This overloaded method performs a serial data read operation. The successfully read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readData(){
    return this->readData(0, false);
}

/**
 * @brief Reads serial data until the desired start bytes are found.
 *
 * This function performs a serial data read operation until the specified start bytes are detected. Any serial data read before the start bytes are found is automatically discarded. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param startBytes A pointer to the start bytes data to be detected.
 * @param sz The size of the start bytes data to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readStartBytes(const unsigned char *startBytes, size_t sz){
    size_t i = 0;
    size_t idxCheck = 0;
    bool found = false;
    int ret = 0;
    std::vector <unsigned char> tmp;
    bool isRcvFirstBytes = false;
    do {
        if (this->remainingData.size() > 0){
            this->data.assign(this->remainingData.begin(), this->remainingData.end());
            this->remainingData.clear();
            ret = 0;
        }
        else {
            ret = this->readData(sz, true);
        }
        if (!ret){
            if (isRcvFirstBytes == false){
                isRcvFirstBytes = true;
            }
            else if (tmp.size() > sz){
                idxCheck = tmp.size() + 1 - sz;
            }
            tmp.insert(tmp.end(), this->data.begin(), this->data.end());
            if (this->remainingData.size() > 0){
                tmp.insert(tmp.end(), this->remainingData.begin(), this->remainingData.end());
                this->remainingData.clear();
            }
            if (tmp.size() >= sz){
                for (i = idxCheck; i <= tmp.size() - sz; i++){
                    if (memcmp(tmp.data() + i, startBytes, sz) == 0){
                        found = true;
                        break;
                    }
                }
            }
        }
    } while(found == false && ret == 0);
    if (found == true){
        this->data.clear();
        this->data.assign(tmp.begin() + i, tmp.begin() + i + sz);
        if (tmp.size() > i + sz) this->remainingData.assign(tmp.begin() + i + sz, tmp.end());
    }
    else {
        this->data.assign(tmp.begin(), tmp.end());
    }
    return ret;
}

/**
 * @brief Overloaded method for `readStartBytes` with `const char*` input.
 *
 * This overloaded function performs a serial data read operation until the specified start bytes are detected. Any serial data read before the start bytes are found is automatically discarded. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param startBytes A pointer to the start bytes data to be detected, provided as a `const char*`.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readStartBytes(const char *startBytes){
    return this->readStartBytes((const unsigned char *) startBytes, strlen(startBytes));
}

/**
 * @brief Overloaded method for `readStartBytes` with `std::vector<unsigned char>` input.
 *
 * This overloaded function performs a serial data read operation until the specified start bytes are detected. Any serial data read before the start bytes are found is automatically discarded. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param startBytes A vector containing the start bytes data to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readStartBytes(const std::vector <unsigned char> startBytes){
    return this->readStartBytes(startBytes.data(), startBytes.size());
}

/**
 * @brief Overloaded method for `readStartBytes` with `std::string` input.
 *
 * This overloaded function performs a serial data read operation until the specified start bytes are detected. Any serial data read before the start bytes are found is automatically discarded. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param startBytes A string containing the start bytes data to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readStartBytes(const std::string startBytes){
    return this->readStartBytes((const unsigned char *) startBytes.c_str(), startBytes.length());
}

/**
 * @brief Performs a serial data read operation until the specified stop bytes are detected.
 *
 * This function reads serial data until the specified stop bytes are detected. Any serial data read up to and including the stop bytes is automatically stored in the buffer. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes The data representing the stop bytes to be detected.
 * @param sz The size of the stop bytes data to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readUntilStopBytes(const unsigned char *stopBytes, size_t sz){
    size_t i = 0;
    size_t idxCheck = 0;
    bool found = false;
    int ret = 0;
    std::vector <unsigned char> tmp;
    bool isRcvFirstBytes = false;
    do {
        if (this->remainingData.size() > 0){
            this->data.assign(this->remainingData.begin(), this->remainingData.end());
            this->remainingData.clear();
            ret = 0;
        }
        else {
            ret = this->readData(sz, true);
        }
        if (!ret){
            if (isRcvFirstBytes == false){
                isRcvFirstBytes = true;
            }
            else if (tmp.size() > sz){
                idxCheck = tmp.size() + 1 - sz;
            }
            tmp.insert(tmp.end(), this->data.begin(), this->data.end());
            if (this->remainingData.size() > 0){
                tmp.insert(tmp.end(), this->remainingData.begin(), this->remainingData.end());
                this->remainingData.clear();
            }
            if (tmp.size() >= sz){
                for (i = idxCheck; i <= tmp.size() - sz; i++){
                    if (memcmp(tmp.data() + i, stopBytes, sz) == 0){
                        found = true;
                        break;
                    }
                }
            }
        }
    } while(found == false && ret == 0);
    if (tmp.size() < sz){
        this->data.assign(tmp.begin(), tmp.end());
        return 2;
    }
    if (this->data.size() != tmp.size()){
        this->data.clear();
        this->data.assign(tmp.begin(), tmp.begin() + i + sz);
        if (tmp.size() > i + sz) this->remainingData.assign(tmp.begin() + i + sz, tmp.end());
        return 0;
    }
    if (this->data.size() > i + sz){
        this->remainingData.assign(this->data.begin() + i + sz, this->data.end());
        this->data.erase(this->data.begin() + i + sz, this->data.end());
    }
    return ret;
}

/**
 * @brief Overloaded function for `readUntilStopBytes` with input as `const char*`.
 *
 * This function reads serial data until the specified stop bytes are detected. Any serial data read up to and including the stop bytes is automatically stored in the buffer. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A pointer to a null-terminated character array representing the stop bytes to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readUntilStopBytes(const char *stopBytes){
    return this->readUntilStopBytes((const unsigned char *) stopBytes, strlen(stopBytes));
}

/**
 * @brief Overloaded function for `readUntilStopBytes` with input as `std::vector<unsigned char>`.
 *
 * This function reads serial data until the specified stop bytes are detected. Any serial data read up to and including the stop bytes is automatically stored in the buffer. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A vector of `unsigned char` representing the stop bytes to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readUntilStopBytes(const std::vector <unsigned char> stopBytes){
    return this->readUntilStopBytes(stopBytes.data(), stopBytes.size());
}

/**
 * @brief Overloaded function for `readUntilStopBytes` with input as `std::string`.
 *
 * This function reads serial data until the specified stop bytes are detected. Any serial data read up to and including the stop bytes is automatically stored in the buffer. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A string representing the stop bytes to be detected.
 * @return `0` if the operation is successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readUntilStopBytes(const std::string stopBytes){
    return this->readUntilStopBytes((const unsigned char *) stopBytes.c_str(), stopBytes.length());
}

/**
 * @brief Reads serial data and checks if the data matches the specified stop bytes.
 *
 * This method performs serial data reading while simultaneously checking if the data matches the desired stop bytes. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A pointer to the stop bytes data to be detected.
 * @param sz The size of the stop bytes data to be detected.
 * @return `0` if the operation is successful and the data is valid.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 * @return `3` if data is read but does not match the specified stop bytes.
 */
int Serial::readStopBytes(const unsigned char *stopBytes, size_t sz){
    bool found = false;
    int ret = 0;
    std::vector <unsigned char> tmp;
    do {
        if (this->remainingData.size() > 0){
            this->data.assign(this->remainingData.begin(), this->remainingData.end());
            this->remainingData.clear();
            ret = 0;
        }
        else {
            ret = this->readData(sz - tmp.size(), true);
        }
        if (!ret){
            tmp.insert(tmp.end(), this->data.begin(), this->data.end());
            if (this->remainingData.size() > 0){
                tmp.insert(tmp.end(), this->remainingData.begin(), this->remainingData.end());
                this->remainingData.clear();
            }
            if (tmp.size() >= sz){
                if (memcmp(tmp.data(), stopBytes, sz) == 0){
                    found = true;
                }
                break;
            }
        }
    } while(ret == 0);
    if (tmp.size() < sz){
        this->data.assign(tmp.begin(), tmp.end());
        return 2;
    }
    if (found == false){
        this->data.assign(tmp.begin(), tmp.end());
        return 3;
    }
    if (this->data.size() != tmp.size()){
        this->data.clear();
        this->data.assign(tmp.begin(), tmp.begin() + sz);
        if (tmp.size() > sz) this->remainingData.assign(tmp.begin() + sz, tmp.end());
        return 0;
    }
    if (this->data.size() > sz){
        this->remainingData.assign(this->data.begin() + sz, this->data.end());
        this->data.erase(this->data.begin() + sz, this->data.end());
    }
    return ret;
}

/**
 * @brief Function overloading for `readStopBytes` with input using `const char*`.
 *
 * This method performs serial data reading while simultaneously checking if the data matches the specified stop bytes. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A C-style string (null-terminated) representing the stop bytes to be detected.
 * @return `0` if the operation is successful and the data is valid.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 * @return `3` if data is read but does not match the specified stop bytes.
 */
int Serial::readStopBytes(const char *stopBytes){
    return this->readStopBytes((const unsigned char *) stopBytes, strlen(stopBytes));
}

/**
 * @brief Function overloading for `readStopBytes` with input using `std::vector`.
 *
 * This method performs serial data reading while simultaneously checking if the data matches the specified stop bytes. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A `std::vector` containing the stop bytes data to be detected.
 * @return `0` if the operation is successful and the data is valid.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 * @return `3` if data is read but does not match the specified stop bytes.
 */
int Serial::readStopBytes(const std::vector <unsigned char> stopBytes){
    return this->readStopBytes(stopBytes.data(), stopBytes.size());
}

/**
 * @brief Function overloading for `readStopBytes` with input using `std::string`.
 *
 * This method performs serial data reading while simultaneously checking if the data matches the specified stop bytes. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param stopBytes A `std::string` object representing the stop bytes to be detected.
 * @return `0` if the operation is successful and the data is valid.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 * @return `3` if data is read but does not match the specified stop bytes.
 */
int Serial::readStopBytes(const std::string stopBytes){
    return this->readStopBytes((const unsigned char *) stopBytes.c_str(), stopBytes.length());
}

/**
 * @brief Performs serial data reading until the desired amount of data is received.
 *
 * This function performs the operation of reading serial data until the specified amount of data is fulfilled. The operation retries up to 3 times, starting from the first data received. The read serial data can be accessed using the `Serial::getBuffer` method.
 *
 * @param sz The size of the serial data to be read.
 * @return `0` if successful.
 * @return `1` if the port is not open.
 * @return `2` if a timeout occurs.
 */
int Serial::readNBytes(size_t sz){
    size_t i = 0;
    std::vector <unsigned char> tmp;
    int ret = 0;
    int tryTimes = 0;
    bool isRcvFirstBytes = false;
    do {
        if (this->remainingData.size() > 0){
            this->data.assign(this->remainingData.begin(), this->remainingData.end());
            this->remainingData.clear();
            ret = 0;
        }
        else {
            ret = this->readData(sz, true);
        }
        if (!ret){
            if (isRcvFirstBytes == false){
                tryTimes = 3;
                isRcvFirstBytes = true;
            }
            tmp.insert(tmp.end(), this->data.begin(), this->data.end());
            if (this->remainingData.size() > 0){
                tmp.insert(tmp.end(), this->remainingData.begin(), this->remainingData.end());
                this->remainingData.clear();
            }
            if (tmp.size() >= sz) break;
        }
        else if (isRcvFirstBytes == true) {
            tryTimes--;
        }
    } while(tryTimes > 0);
    if (tmp.size() < sz){
        this->data.assign(tmp.begin(), tmp.end());
        return 2;
    }
    if (this->data.size() != tmp.size()){
        this->data.clear();
        this->data.assign(tmp.begin(), tmp.begin() + sz);
        if (tmp.size() > sz) this->remainingData.assign(tmp.begin() + sz, tmp.end());
        return 0;
    }
    if (this->data.size() > sz){
        this->remainingData.assign(this->data.begin() + sz, this->data.end());
        this->data.erase(this->data.begin() + sz, this->data.end());
    }
    return 0;
}

/**
 * @brief Retrieves the amount of successfully read data.
 *
 * This function retrieves the information about the size of the data that has been successfully read.
 *
 * @return The size of the serial data in bytes.
 */
size_t Serial::getDataSize(){
    return this->data.size();
}

/**
 * @brief Retrieves the read data buffer.
 *
 * This function retrieves all the data that has been successfully read by the `read` method.
 *
 * @param buffer A variable to hold the serial data that has been successfully read.
 * @param maxBufferSz The maximum size of the data that can be accommodated in the buffer.
 * @return The size of the serial data read.
 */
size_t Serial::getBuffer(unsigned char *buffer, size_t maxBufferSz){
    pthread_mutex_lock(&(this->mtx));
    size_t result = (this->data.size() < maxBufferSz ? this->data.size() : maxBufferSz);
    size_t sz = result;
    for (auto i = this->data.begin(); i != this->data.end(); i++){
        if (result == 0) break;
        *buffer = *i;
        buffer++;
        result--;
    }
    pthread_mutex_unlock(&(this->mtx));
    return sz;
}

/**
 * @brief Overloaded method for `getBuffer` with `std::vector<unsigned char>` as the output parameter.
 *
 * Retrieves all the data that has been successfully read by the `read` method.
 *
 * This overload allows you to use a `std::vector<unsigned char>` as the buffer to hold the serial data.
 *
 * @param buffer A `std::vector<unsigned char>` to hold the serial data that has been successfully read.
 * @return The size of the serial data read.
 */
size_t Serial::getBuffer(std::vector <unsigned char> &buffer){
    buffer.clear();
    buffer.assign(this->data.begin(), this->data.end());
    return buffer.size();
}

/**
 * @brief Retrieves the read data buffer as a vector.
 *
 * This method returns all the data that has been successfully read by the `read` method as a `std::vector<unsigned char>`.
 *
 * @return A `std::vector<unsigned char>` containing the serial data that has been successfully read.
 */
std::vector <unsigned char> Serial::getBufferAsVector(){
    std::vector <unsigned char> tmp;
    tmp.assign(this->data.begin(), this->data.end());
    return tmp;
}

/**
 * @brief Retrieves the number of bytes in the remaining buffer.
 *
 * This method provides the size of the data that is still present in the remaining buffer.
 *
 * @return The size of the remaining data in bytes.
 */
size_t Serial::getRemainingDataSize(){
    return this->remainingData.size();
}

/**
 * @brief Retrieves the remaining serial data read outside of the data buffer.
 *
 * This method extracts all remaining serial data that has been successfully read but is outside the main data buffer.
 *
 * @param buffer Variable to hold the remaining serial data read.
 * @param maxBufferSz Maximum size of data that can be held by the buffer variable.
 * @return The size of the serial data read.
 */
size_t Serial::getRemainingBuffer(unsigned char *buffer, size_t maxBufferSz){
    pthread_mutex_lock(&(this->mtx));
    size_t result = (this->remainingData.size() < maxBufferSz ? this->remainingData.size() : maxBufferSz);
    size_t sz = result;
    for (auto i = this->remainingData.begin(); i != this->remainingData.end(); i++){
        if (result == 0) break;
        *buffer = *i;
        buffer++;
        result--;
    }
    pthread_mutex_unlock(&(this->mtx));
    return sz;
}

/**
 * @brief Overloading method for __getRemainingBuffer__ with output parameter as vector.
 *
 * Retrieves all remaining serial data that has been successfully read but is outside the main data buffer.
 *
 * @param buffer Variable to hold the remaining serial data read, provided as a vector.
 * @return The size of the remaining serial data read.
 */
size_t Serial::getRemainingBuffer(std::vector <unsigned char> &buffer){
    buffer.clear();
    buffer.assign(this->remainingData.begin(), this->remainingData.end());
    return buffer.size();
}

/**
 * @brief Retrieves remaining serial data that has been successfully read but is outside the main data buffer, returning it as a vector.
 *
 * This method retrieves all remaining serial data that has been read successfully but is not included in the main data buffer, and returns it as a vector.
 *
 * @return std::vector<unsigned char> containing the remaining serial data that has been successfully read.
 */
std::vector <unsigned char> Serial::getRemainingBufferAsVector(){
    std::vector <unsigned char> tmp;
    tmp.assign(this->remainingData.begin(), this->remainingData.end());
    return tmp;
}

/**
 * @brief Performs the operation of writing serial data.
 *
 * This method writes the specified data to the serial port.
 *
 * @param buffer Data to be written.
 * @param sz Size of the data to be written.
 * @return 0 if the operation is successful.
 * @return 1 if the port is not open.
 * @return 2 if the data write operation fails.
 */
int Serial::writeData(const unsigned char *buffer, size_t sz){
    pthread_mutex_lock(&(this->wmtx));
    size_t total = 0;
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd <= 0 || this->usb == nullptr){
        pthread_mutex_unlock(&(this->wmtx));
        return 1;
    }
    ssize_t bytes = 0;
#else
    long unsigned int bytes = 0;
#endif
    while (total < sz){
#if defined(PLATFORM_POSIX) || defined(__linux__)
        if (this->usb == nullptr){
            bytes = write(this->fd, (void *) (buffer + total), sz - total);
        }
        else {
            bytes = this->usb->writeDevice(buffer + total, sz - total);
        }
#else
        bool success = WriteFile(this->fd, (buffer + total), sz - total, &bytes, NULL);
        if (success == false){
            bytes = 0;
        }
#endif
        if (bytes > 0){
            total += bytes;
        }
        else {
            pthread_mutex_unlock(&(this->wmtx));
            return 2;
        }
    }
    pthread_mutex_unlock(&(this->wmtx));
    return 0;
}

/**
 * @brief Method overloading of `writeData` with input as `const char*`.
 *
 * This method writes the specified data to the serial port. This overload allows the data to be passed as a `const char*`.
 *
 * @param buffer Data to be written.
 * @return 0 if the operation is successful.
 * @return 1 if the port is not open.
 * @return 2 if the data write operation fails.
 */
int Serial::writeData(const char *buffer){
    return this->writeData((const unsigned char *) buffer, strlen(buffer));
}

/**
 * @brief Method overloading of `writeData` with input as `std::vector <unsigned char>`.
 *
 * This method writes the specified data to the serial port. This overload allows the data to be passed as a `std::vector <unsigned char>`.
 *
 * @param buffer Data to be written.
 * @return 0 if the operation is successful.
 * @return 1 if the port is not open.
 * @return 2 if the data write operation fails.
 */
int Serial::writeData(const std::vector <unsigned char> buffer){
    return this->writeData(buffer.data(), buffer.size());
}

/**
 * @brief Method overloading of `writeData` with input as `std::string`.
 *
 * This method writes the specified data to the serial port. This overload allows the data to be passed as a `std::string`.
 *
 * @param buffer Data to be written.
 * @return 0 if the operation is successful.
 * @return 1 if the port is not open.
 * @return 2 if the data write operation fails.
 */
int Serial::writeData(const std::string buffer){
    return this->writeData((const unsigned char *) buffer.c_str(), buffer.length());
}

/**
 * @brief Closes the serial communication port.
 *
 * This function is used to close the currently open serial communication port,
 * ensuring that the port is no longer in use and that any associated system resources
 * are released.
 */
void Serial::closePort(){
    pthread_mutex_lock(&(this->wmtx));
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->usb == nullptr){
        if (this->fd > 0) close(this->fd);
        this->fd = -1;
    }
    else {
        this->usb->closeDevice();
    }
#else
    CloseHandle(this->fd);
#endif
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_unlock(&(this->wmtx));
}
