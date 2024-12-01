/*
 * $Id: serial.hpp,v 1.0.0 2024/08/25 13:03:53 Jaya Wikrama Exp $
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

/**
 * @file
 * @brief Enhanced Serial Communication Functions.
 *
 * This file contains a collection of functions and commands designed to facilitate and extend
 * basic serial communication in C++. These functions are intended to simplify the process of
 * setting up, sending, receiving, and managing data over serial connections. The enhancements
 * provided in this header file go beyond the standard library functions, offering more
 * flexibility and control for developers working with serial interfaces.
 *
 * The key functionalities include:
 * - Initialization and configuration of serial ports.
 * - Sending and receiving data over serial connections.
 * - Error handling and diagnostics for serial communication.
 * - Utility functions for managing serial buffers and flow control.
 *
 * The functions in this file are designed to be easy to integrate into various projects,
 * providing a robust foundation for serial communication in embedded systems, networking,
 * or any application that requires serial data transmission.
 *
 * @note This file is a part of a larger project focusing on enhancing serial communication
 *       capabilities in C++ applications.
 *
 * @version 1.0.0
 * @date 2024-08-25
 * @author Jaya Wikrama
 */

#ifndef __SERIAL_BASIC_HPP__
#define __SERIAL_BASIC_HPP__

#include "usb-serial.hpp"
#include <vector>
#if defined(PLATFORM_POSIX) || defined(__linux__)
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#else
#undef bytes
#include <windows.h>
#include <stdint.h>
/**
 * @note On Windows, this implementation is created without using Mutex and thread functionality.
 */
typedef int pthread_mutex_t;
int pthread_mutex_init(pthread_mutex_t *mtx, void *ptr){
  // do noting
  return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mtx){
  // do noting
  return 0;
}
int pthread_mutex_unlock(pthread_mutex_t *mtx){
  // do noting
  return 0;
}

typedef enum _speed_t {
  B0 = 0,
  B50 = 50,
  B75 = 75,
  B110 = 110,
  B134 = 134,
  B150 = 150,
  B200 = 200,
  B300 = 300,
  B600 = 600,
  B1200 = 1200,
  B1800 = 1800,
  B2400 = 2400,
  B4800 = 4800,
  B9600 = 9600,
  B19200 = 19200,
  B38400 = 38400,
  B57600 = 57600,
  B115200 = 115200,
  B230400 = 230400,
  B460800 = 460800,
  B192600 = 192600
} speed_t;

#endif
#include <string>

class Serial {
  private:
#if defined(PLATFORM_POSIX) || defined(__linux__)
    int fd;
#else
    HANDLE fd;
#endif
    speed_t baud;
    unsigned int timeout;
    unsigned int keepAliveMs;
    std::string port;
    pthread_mutex_t mtx;
    pthread_mutex_t wmtx;
  protected:
    USBSerial *usb;
    std::vector <unsigned char> data;
    std::vector <unsigned char> remainingData;
    /**
     * @brief Sets the file descriptor.
     *
     * This setter function assigns a value to the file descriptor.
     *
     * @param fd The file descriptor to be set.
     */
#if defined(PLATFORM_POSIX) || defined(__linux__)
    void setFileDescriptor(int fd);
#else
    void setFileDescriptor(HANDLE fd);
#endif

    /**
     * @brief Configures serial port attributes.
     *
     * This function sets up or configures the attributes of the file descriptor for a successfully opened serial port.
     *
     * @return true if the configuration is successful.
     * @return false if the configuration fails.
     */
    bool setupAttributes();
  public:
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
    Serial();

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
    Serial(const char *port, speed_t baud, unsigned int timeout);

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
    Serial(const std::string port, speed_t baud, unsigned int timeout);

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
    Serial(const std::string port, speed_t baud, unsigned int timeout, unsigned int keepAliveMs);

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
    Serial(const char *port, speed_t baud, unsigned int timeout, unsigned int keepAliveMs);

    /**
     * @brief Custom constructor.
     *
     * This constructor is used for specific purposes where the source device uses USB directly.
     *
     * @param usb The pointer of USB Serial Object.
     */
    Serial(USBSerial *usb);

    /**
     * @brief Destructor.
     *
     * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
     * It ensures that all allocated resources are properly freed, preventing memory leaks.
     */
    ~Serial();

    /**
     * @brief Sets link to USB Device pointer.
     *
     * This setter function configures USB device as serial device.
     *
     * @param usb The USB Serial device pointer.
     */
    void setUSBDevice(USBSerial *usb);

    /**
     * @brief Sets the serial port device.
     *
     * This setter function configures the serial port device to be used for communication.
     *
     * @param port The serial port device (e.g., "/dev/ttyUSB0").
     */
    void setPort(const std::string port);

    /**
     * @brief Sets the baud rate for communication.
     *
     * This setter function configures the baud rate used for serial communication.
     *
     * @param baud The baud rate (e.g., `B9600` for 9600 bps).
     */
    void setBaudrate(speed_t baud);

    /**
     * @brief Sets the communication timeout.
     *
     * This setter function configures the timeout for serial communication. The timeout value is specified in units of 100 milliseconds.
     *
     * @param timeout The timeout value (e.g., `10` for a 1-second timeout).
     */
    void setTimeout(unsigned int timeout);

    /**
     * @brief Sets the keep-alive interval for communication.
     *
     * This setter function configures the maximum wait time for receiving the next byte of serial data after the initial byte has been received. This helps maintain the connection by ensuring timely data reception.
     *
     * @param keepAliveMs The keep-alive interval in milliseconds.
     */
    void setKeepAlive(unsigned int keepAliveMs);

    /**
     * @brief Gets the serial port.
     *
     * This getter function retrieves the serial port currently being used.
     *
     * @return The serial port as a string (e.g., "/dev/ttyUSB0").
     */
    std::string getPort();

    /**
     * @brief Gets the baud rate for serial communication.
     *
     * This getter function retrieves the baud rate currently configured for serial communication.
     *
     * @return The baud rate (e.g., `B9600` for 9600 bps).
     */
    speed_t getBaudrate();

    /**
     * @brief Gets the communication timeout.
     *
     * This getter function retrieves the timeout value configured for serial communication. The timeout is specified in units of 100 milliseconds.
     *
     * @return The timeout value (e.g., `10` for a 1-second timeout).
     */
    unsigned int getTimeout();

    /**
     * @brief Gets the keep-alive interval for communication.
     *
     * This getter function retrieves the maximum wait time configured for receiving the next byte of serial data after the initial byte has been successfully received. The interval is specified in milliseconds.
     *
     * @return The keep-alive interval in milliseconds.
     */
    unsigned int getKeepAlive();

    /**
     * @brief Gets the file descriptor.
     *
     * This getter function retrieves the current value of the file descriptor.
     *
     * @return The current file descriptor.
     */
#if defined(PLATFORM_POSIX) || defined(__linux__)
    int getFileDescriptor();
#else
    HANDLE getFileDescriptor();
#endif

    /**
     * @brief Opens the serial port for communication.
     *
     * This function attempts to open the specified serial port for communication. It configures the port according to the current settings and prepares it for data transfer.
     *
     * @return 0 if the port is successfully opened.
     * @return 1 if the port fails to open.
     */
    int openPort();

#if defined(PLATFORM_POSIX) || defined(__linux__)
    /**
     * @brief Checks for available input bytes.
     *
     * This function checks whether there are any bytes available in the serial buffer for reading.
     *
     * @return `true` if there are bytes available in the serial buffer.
     * @return `false` if there are no bytes available in the serial buffer.
     */
    bool isInputBytesAvailable();
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
    int readData(size_t sz, bool dontSplitRemainingData);

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
    int readData(size_t sz);

    /**
     * @brief Overloaded method for `readData` to perform serial data reading.
     *
     * This overloaded method performs a serial data read operation. The successfully read serial data can be accessed using the `Serial::getBuffer` method.
     *
     * @return `0` if the operation is successful.
     * @return `1` if the port is not open.
     * @return `2` if a timeout occurs.
     */
    int readData();

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
    int readStartBytes(const unsigned char *startBytes, size_t sz);

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
    int readStartBytes(const char *startBytes);

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
    int readStartBytes(const std::vector <unsigned char> startBytes);

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
    int readStartBytes(const std::string startBytes);

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
    int readUntilStopBytes(const unsigned char *stopBytes, size_t sz);

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
    int readUntilStopBytes(const char *stopBytes);

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
    int readUntilStopBytes(const std::vector <unsigned char> stopBytes);

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
    int readUntilStopBytes(const std::string stopBytes);

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
    int readStopBytes(const unsigned char *stopBytes, size_t sz);

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
    int readStopBytes(const char *stopBytes);

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
    int readStopBytes(const std::vector <unsigned char> stopBytes);

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
    int readStopBytes(const std::string stopBytes);

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
    int readNBytes(size_t sz);

    /**
     * @brief Retrieves the amount of successfully read data.
     *
     * This function retrieves the information about the size of the data that has been successfully read.
     *
     * @return The size of the serial data in bytes.
     */
    size_t getDataSize();

    /**
     * @brief Retrieves the read data buffer.
     *
     * This function retrieves all the data that has been successfully read by the `read` method.
     *
     * @param buffer A variable to hold the serial data that has been successfully read.
     * @param maxBufferSz The maximum size of the data that can be accommodated in the buffer.
     * @return The size of the serial data read.
     */
    size_t getBuffer(unsigned char *buffer, size_t maxBufferSz);

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
    size_t getBuffer(std::vector <unsigned char> &buffer);

    /**
     * @brief Retrieves the read data buffer as a vector.
     *
     * This method returns all the data that has been successfully read by the `read` method as a `std::vector<unsigned char>`.
     *
     * @return A `std::vector<unsigned char>` containing the serial data that has been successfully read.
     */
    std::vector <unsigned char> getBufferAsVector();

    /**
     * @brief Retrieves the number of bytes in the remaining buffer.
     *
     * This method provides the size of the data that is still present in the remaining buffer.
     *
     * @return The size of the remaining data in bytes.
     */
    size_t getRemainingDataSize();

    /**
     * @brief Retrieves the remaining serial data read outside of the data buffer.
     *
     * This method extracts all remaining serial data that has been successfully read but is outside the main data buffer.
     *
     * @param buffer Variable to hold the remaining serial data read.
     * @param maxBufferSz Maximum size of data that can be held by the buffer variable.
     * @return The size of the serial data read.
     */
    size_t getRemainingBuffer(unsigned char *buffer, size_t maxBufferSz);

    /**
     * @brief Overloading method for __getRemainingBuffer__ with output parameter as vector.
     *
     * Retrieves all remaining serial data that has been successfully read but is outside the main data buffer.
     *
     * @param buffer Variable to hold the remaining serial data read, provided as a vector.
     * @return The size of the remaining serial data read.
     */
    size_t getRemainingBuffer(std::vector <unsigned char> &buffer);

    /**
     * @brief Retrieves remaining serial data that has been successfully read but is outside the main data buffer, returning it as a vector.
     *
     * This method retrieves all remaining serial data that has been read successfully but is not included in the main data buffer, and returns it as a vector.
     *
     * @return std::vector<unsigned char> containing the remaining serial data that has been successfully read.
     */
    std::vector <unsigned char> getRemainingBufferAsVector();

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
    int writeData(const unsigned char *buffer, size_t sz);

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
    int writeData(const char *buffer);

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
    int writeData(const std::vector <unsigned char> buffer);

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
    int writeData(const std::string buffer);

    /**
     * @brief Closes the serial communication port.
     *
     * This function is used to close the currently open serial communication port,
     * ensuring that the port is no longer in use and that any associated system resources
     * are released.
     */
    void closePort();
};

#endif
