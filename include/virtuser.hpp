/*
 * $Id: virtuser.hpp,v 1.0.0 2024/07/29 18:41:03 Jaya Wikrama Exp $
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
 * @file virtuser.hpp
 * @brief Header file for virtual serial port functionality and handler functions.
 *
 * This file defines a set of functions and classes for creating and managing virtual
 * serial ports. The functionalities provided allow for the simulation of serial
 * communication interfaces, which can be useful for testing, debugging, or developing
 * applications that interact with serial ports without requiring physical hardware.
 *
 * The key components included in this file are:
 * - Definitions and classes for virtual serial ports.
 * - Functions to create, configure, and manage virtual serial ports.
 * - Handler functions to manage data flow and events on virtual serial ports.
 * - Utility functions for simulating serial communication scenarios and testing.
 *
 * The virtual serial ports provided by this file can be used to simulate communication
 * between software components or to test applications that rely on serial data exchange.
 * The handler functions offer control over how data is processed and how events are
 * handled, providing a flexible and powerful toolset for developers.
 *
 * @note This file is intended for use in environments where physical serial ports are
 *       not available or practical. It is particularly useful in development and
 *       testing scenarios.
 *
 * @version 1.0.0
 * @date 2024-07-29
 * @author Jaya Wikrama
 */

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
    VirtualSerial();

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
    VirtualSerial(speed_t baud, unsigned int timeout, unsigned int keepAliveMs);

    /**
     * @brief Destructor.
     *
     * Releases all allocated memory and closes the master port of the virtual serial port.
     */
    ~VirtualSerial();

    /**
     * @brief Sets the callback function for read and write operations on the master port of the virtual serial port.
     *
     * This method allows you to specify a callback function that will be used for handling data operations
     * (reading and writing) on the virtual serial port's master port.
     *
     * @param func Pointer to the callback function.
     * @param param Pointer to the parameter for the callback function.
     */
    void setCallback(const void *func, void *param);

    /**
     * @brief Gets the callback function used for read and write operations on the master port of the virtual serial port.
     *
     * @return Pointer to the callback function.
     */
    const void *getCallbackFunction();

    /**
     * @brief Gets the parameter pointer used for the callback function in read and write operations on the master port of the virtual serial port.
     *
     * @return Pointer to the parameter of the callback function.
     */
    void *getCallbackParam();

    /**
     * @brief Gets the name of the virtual serial port.
     *
     * @return The name of the virtual serial port (slave).
     */
    std::string getVirtualPortName();

    /**
     * @brief Method to start executing the callback function.
     *
     * This method initiates the execution of the previously set callback function.
     *
     * @return `false` if the callback function has not been set up.
     * @return `true` if the callback function has been successfully executed.
     */
    bool begin();
};

#endif