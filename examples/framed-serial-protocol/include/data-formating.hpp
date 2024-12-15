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

/**
 * @file
 * @brief Access Protocol Format to <your-module-or-device>.
 *
 * This file contains the standard format used by <your-module-or-device>.
 * The classes contained in this file are useful for simplifying the preparation
 * and parsing process of serial data.
 *
 * The key functionalities include:
 * - Protocol Interface of <your-module-or-device>.
 * - Using part of Serialink Library to manage frame format.
 *
 * You can access Serialink Library at: https://github.com/JayaWikrama/Serialink
 *
 * @note This file is a part of a larger project focusing on <your-project>.
 *
 * @version 1.0.0
 * @date 2024-12-09
 * @author Jaya Wikrama
 */

#ifndef __RQX_FORMAT_HPP__
#define __RQX_FORMAT_HPP__

#include "serialink.hpp"
#include <vector>

class ProtocolFormat {
  private:
    DataFrame *frameProtocol;

  public:
    /**
     * @brief Constructor of ProtocolFormat object.
     *
     * This constructor initializes the Protocol or Data Frame Format of <your-module-or-device>.
     * If there is a failure while initializing the object, the method will throw a
     * runtime_error signal which you can retrieve in the try exception mechanism.
     *
     * @param obj The main object of serial handler.
     */
    ProtocolFormat(Serialink &obj);

    /**
     * @brief Destructor.
     *
     * This destructor is responsible for releasing any memory that has been allocated during the object's lifetime.
     * It ensures that all allocated resources are properly freed, preventing memory leaks.
     */
    ~ProtocolFormat();

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
    static void setDataLength(DataFrame &frame, void *ref);

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
    static void validate(DataFrame &frame, void *ref);

    /**
     * @brief Framed data builder.
     *
     * This method functions is responsible to build framed data that ready to be send/write to serial devices. 
     *
     * @param data The main data of protocol.
     */
    std::vector <unsigned char> buildCommand(const unsigned char *data, size_t sz);

    /**
     * @brief Overloading method of framed data builder.
     *
     * This method functions is responsible to build framed data that ready to be send/write to serial devices. 
     *
     * @param data The main data of protocol.
     */
    std::vector <unsigned char> buildCommand(const std::vector <unsigned char> &data);

    /**
     * @brief Display Hex data.
     *
     * This method functions is responsible to display hexadecimal data. 
     */
    static void displayData(const unsigned char *data, size_t sz);

    /**
     * @brief Overloading method of Hex data display.
     *
     * This method functions is responsible to display hexadecimal data. 
     */
    static void displayData(const std::vector <unsigned char> &data);
};

#endif
