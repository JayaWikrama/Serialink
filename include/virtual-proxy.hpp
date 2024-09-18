/*
 * $Id: virtal-proxy.hpp,v 1.0.0 2024/09/18 12:56:03 Jaya Wikrama Exp $
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
 * @file virtal-proxy.hpp
 * @brief Header file for virtual serial proxy functionality and handler functions.
 *
 * The Virtual Serial Proxy is a software intermediary that connects physical serial devices to the
 * main application through a virtual serial port. This proxy is designed to facilitate easier control
 * and monitoring of data traffic between the application and the serial device. By using the Virtual
 * Serial Proxy, users can manage serial communication without needing direct physical access to the
 * serial port, simplifying the process of interacting with the device.
 *
 * Key Functions:
 * 1. Transparent Connection
 *    The proxy bridges communication between the physical serial device and the application, allowing
 *    the application to interact with the device as if it were connected to a local physical port.
 * 2. Data Traffic Monitoring
 *    Provides enhanced visibility into the flow of data between the application and the serial device,
 *    enabling better monitoring and control.
 * 3. Simplified User Management
 *    Makes it easier for users to manage devices remotely or locally, without requiring extensive
 *    configuration or physical connections.
 *
 * The Virtual Serial Proxy is ideal for scenarios where managing multiple devices and monitoring data
 * flow is essential, offering a more flexible and streamlined approach to serial communication.
 *
 *
 * @version 1.0.0
 * @date 2024-09-18
 * @author Jaya Wikrama
 */

#ifndef __VIRTUAL_PROXY_DEVICE_HPP__
#define __VIRTUAL_PROXY_DEVICE_HPP__

#include "virtuser.hpp"

class VirtualSerialProxy {
  private:
    speed_t workingBaudrate;
    std::string physicalPort;
    std::string symlinkPort;
    VirtualSerial *pty;
    Serial *dev;
    const void *passthroughFunc;
    void *passthroughParam;
  public:
    /**
     * @brief Default constructor.
     *
     * Initializes a virtual serial proxy port.
     *
     * Default parameters:
     * - Baud rate: B9600
     * - Timeout: 10 (1 second)
     * - Keep-alive interval: 10 milliseconds
     */
    VirtualSerialProxy();

    /**
     * @brief Custom constructor.
     *
     * Initializes a virtual serial port with specified settings. The name or address of the virtual port for the slave
     * can be obtained using the `getVirtualPortName` method.
     *
     * @param physicalPort The connected physical serial devices.
     * @param baud The baud rate for the serial communication.
     */
    VirtualSerialProxy(const char *physicalPort, speed_t baud);

    /**
     * @brief Custom constructor.
     *
     * Initializes a virtual serial port with specified settings. The name or address of the virtual port for the slave
     * can be obtained using the `getVirtualPortName` method.
     *
     * @param physicalPort The connected physical serial devices.
     * @param symlinkPort The symbolic link name of pseudo serial (may need to be run as root or require root/sudo password).
     * @param baud The baud rate for the serial communication.
     */
    VirtualSerialProxy(const char *physicalPort, const char *symlinkPort, speed_t baud);

    /**
     * @brief Destructor.
     *
     * Releases all allocated memory and closes the master port of the virtual serial port.
     */
    ~VirtualSerialProxy();

    /**
     * @brief Sets the physical serial port device.
     *
     * This setter function configures the serial port device to be used for communication.
     *
     * @param port The serial port device (e.g., "/dev/ttyUSB0").
     */
    void setPhysicalPort(const std::string port);

    /**
     * @brief Sets the symbolic link of pseudo serial port device.
     *
     * This setter function configures the serial port device to be used for communication.
     *
     * @param port The serial port device (e.g., "/dev/ttyS4").
     */
    void setSymlinkPort(const std::string port);

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
     * @brief Gets the physical serial port.
     *
     * This getter function retrieves the physical serial port currently being used.
     *
     * @return The physical serial port as a string (e.g., "/dev/ttyUSB0").
     */
    std::string getPhysicalPort();

    /**
     * @brief Gets the symbolic name of pseudo serial port.
     *
     * This getter function retrieves the physical serial port currently being used.
     *
     * @return The symbolic name of pseudo serial port (e.g., "/dev/ttyS4") of the name of pseudo serial port (if fail to set symbolic name or symbolic name is not set).
     */
    std::string getSymlinkPort();

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
     * @brief Sets the Pass Through function.
     *
     * This method allows you to specify a callback function that will be used for handling data operations
     * (reading and writing) on the virtual serial poxy. (Mandatory before call the `begin` method).
     *
     * @param func Pass Through function that has 3 parameters. First `Serial &` is source and the second `Serial &` is destination. `void *` is a pointer that will connect directly to `void *param`.
     * @param param Pointer to the parameter for the callback function.
     */
    void setPassThrough(void (*func)(Serial &, Serial &, void *), void *param);

    /**
     * @brief Gets the Pass Through function.
     *
     * @return Pointer to the callback function.
     */
    const void *getPassThroughFunction();

    /**
     * @brief Gets the parameter pointer used for the Pass Through function.
     *
     * @return Pointer to the parameter of the Pass Through function.
     */
    void *getPassThroughParam();

    /**
     * @brief Method to start the proxy.
     *
     * @return `false` if the Pass Through function has not been set up or failed to start operation.
     * @return `true` if the Pass Through function has been successfully executed.
     */
    bool begin();
};

#endif