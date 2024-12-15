/*
 * $Id: serialink.hpp,v 1.0.0 2024/08/27 20:51:47 Jaya Wikrama Exp $
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
 * @brief Advance Serial Protocol (with framed data) library.
 *
 * This library provides a robust implementation of a serial communication protocol
 * that includes data framing. It is designed for reliable data exchange over serial
 * connections, supporting error detection and packet-based communication.
 *
 * @version 1.0.0
 * @date 2024-08-27
 * @author Jaya Wikrama
 */

#ifndef __SERIALLINK_HPP__
#define __SERIALLINK_HPP__

#include "serial.hpp"
#include "data-frame.hpp"
#include "validator.hpp"

class Serialink : public Serial {
  private:
    bool isFormatValid;
    DataFrame *frameFormat;
  public:
    /**
    * @brief Default constructor.
    *
    * Initializes private data members and parameters to their default values.
    */
    Serialink();

    /**
     * @brief Custom constructor.
     *
     * This constructor is used for specific purposes where the source device uses USB directly.
     *
     * @param usb The pointer of USB Serial Object.
     */
    Serialink(USBSerial *usb);

    /**
     * @brief Destructor.
     *
     * Releases any allocated memory.
     */
    ~Serialink();

    /**
     * @brief Retrieves the memory address of the frame format.
     *
     * This function returns the address of the `frameFormat` data member.
     *
     * @return The memory address of the `frameFormat`.
     */
    DataFrame *getFormat();

    /**
     * @brief Stops reading framed serial data.
     *
     * This function sets up variables that act as indicators for the validity of the received serial data,
     * allowing it to stop reading framed serial data from user space. It achieves this by using a post-execution
     * function configured within the `DataFrame` class.
     */
    void trigInvDataIndicator();

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
    int readFramedData();

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
    int writeFramedData();

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
    std::vector <unsigned char> getSpecificBufferAsVector(DataFrame::FRAME_TYPE_t begin, DataFrame::FRAME_TYPE_t end);

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
    std::vector <unsigned char> getSpecificBufferAsVector(const DataFrame *begin, const DataFrame *end);

    Serialink& operator=(const DataFrame &obj);

    Serialink& operator+=(const DataFrame &obj);

    Serialink& operator+(const DataFrame &obj);

    DataFrame* operator[](int idx);

    DataFrame* operator[](DataFrame::FRAME_TYPE_t type);

    DataFrame* operator[](std::pair <DataFrame::FRAME_TYPE_t, int> params);
};

#endif
