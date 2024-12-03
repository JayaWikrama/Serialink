
# Serialink Library

Serialink is a library for serial communication designed to support and simplify software development in Linux environments. This library provides a straightforward interface and configuration, making it easy to integrate into various applications. Additionally, Serialink supports advanced serial communication, including communication using specific protocols (framed data).

## Installation

Follow these steps to install the necessary packages and dependencies.

### Installing Compiler and CMake

1. Install the Compiler:

```bash
sudo apt install build-essential g++ binutils
```

3. Install CMake:

```bash
sudo apt install make cmake
```

### Install USB Library

1. Install libusb-1.0.0

```bash
sudo apt-get install libusb-1.0-0-dev
```

### Installing Google Test

1. Clone repository:

```bash
git clone https://github.com/google/googletest.git -b v1.14.0
```

2. Navigate to the cloned repository directory:

```bash
cd googletest
```

3. Build the Google Test Library:

```bash
mkdir build
cd build
cmake ..
make
```

4. Install Google Test Library:

```bash
sudo make install
```

5. Return to the root directory:

```bash
cd ../../
```

## Building the Library for Test

1. Clone the main repository:

```bash
git clone https://github.com/JayaWikrama/Serialink.git
```

2. Navigate to the project directory:

```bash
cd Serialink
```

3. Initialize and update all Git submodules:

```bash
git submodule init 
git submodule update --init --recursive
```

4. If you want to use the latest source code (same as the remote repository), run:

```bash
git submodule update --remote
```

5. Create a build directory:

```bash
mkdir build
cd build
```

6. Configure CMake:

```bash
cmake -DBUILD_TESTS=ON ..
```

Notes:

`-DBUILD_TESTS=ON` flags for create test apps. If you dont need test apps, just run `cmake ..`.
`-DUSE_USB_SERIAL=ON` flags for activate support to USB Serial direct access.

7. Build the library:

```bash
make
```

8. Run the unit tests to ensure all functions are working correctly:

```bash
./Serialink-test
```

Sample test output:

```bash
[==========] Running 35 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 26 tests from SerialinkSimpleTest
[ RUN      ] SerialinkSimpleTest.DefaultConstructor_1
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkSimpleTest.DefaultConstructor_1 (0 ms)
[ RUN      ] SerialinkSimpleTest.CustomConstructor_1
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkSimpleTest.CustomConstructor_1 (0 ms)
[ RUN      ] SerialinkSimpleTest.CustomConstructor_2
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkSimpleTest.CustomConstructor_2 (0 ms)
....
....
....
[ RUN      ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_1
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_1 (50 ms)
[ RUN      ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_2
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_2 (50 ms)
[ RUN      ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_3
Virtual Serial Name (slave): /dev/pts/1
Virtual serial port ready
[       OK ] SerialinkFramedDataTest.ReadTest_withPrefixAndSuffix_3 (2767 ms)
[----------] 9 tests from SerialinkFramedDataTest (3021 ms total)

[----------] Global test environment tear-down
[==========] 35 tests from 2 test suites ran. (7739 ms total)
[  PASSED  ] 35 tests.
```

## Using the Library

One way to use this library is by integrating it into your main application as a Git submodule. Here’s an example of how to create a new project and integrate the Serialink library into it:

1. Create the project tree.

```bash
mkdir SerialReceiveApp
cd SerialReceiveApp
mkdir src
mkdir include
mkdir external
git init .
```

2. Add the Serialink library as an external library.

```bash
git submodule add https://github.com/JayaWikrama/Serialink.git external/Serialink
git submodule update --init --recursive
```

3. Create the main application source code.

```bash
nano src/main.cpp
```

Here is the sample source code.

```cpp
#include <iostream>
#include <string.h>

#include "serialink.hpp"

const std::string errorMessage[] = {
    "Serial Port Has Not Been Opened",
    "Timeout",
    "Frame Format Has Not Been Setup"
};

void displayData(const std::vector <unsigned char> &data){
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << (int) *i;
        std::cout << " ";
    }
    std::cout << std::endl;
}

void crc16(DataFrame &frame, void *ptr) {
    /* Get Serialink object form function param */
    Serialink *obj = (Serialink *) ptr;
    /* Initialize crc16 param */
    unsigned short crc = 0x0000;
    unsigned short poly = 0x1021;
    /* Get data from Start Bytes until Data */
    std::vector <unsigned char> data = obj->getSpecificBufferAsVector(DataFrame::FRAME_TYPE_START_BYTES, DataFrame::FRAME_TYPE_DATA);
    std::cout << "Data from which the CRC value will be calculated:" << std::endl;
    displayData(data);
    /* Calculate crc16 */
    for (const auto &byte : data) {
        crc ^= (static_cast<unsigned short>(byte) << 8);
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ poly;
            } else {
                crc <<= 1;
            }
        }
    }
    /* Compare received CRC with calculated CRC */
    unsigned short rcvCRC = 0;
    frame.getData((unsigned char *) &rcvCRC, 2);
    if (rcvCRC != crc){
        /* invalid crc -> trigger stop flag to __readFramedData__ method */
        obj->trigInvDataIndicator();
        std::cout << "CRC16 Invalid (0x" << std::hex << rcvCRC << " != 0x" << std::hex << crc << ")" << std::endl;
    }
}

void setupLengthByCommand(DataFrame &frame, void *ptr){
    int data = 0;
    /* Get Serialink object form function param */
    Serialink *obj = (Serialink *) ptr;
    /* Get DataFrame target */
    DataFrame *target = (*obj)[DataFrame::FRAME_TYPE_DATA];
    if (target == nullptr) return;
    frame.getData((unsigned char *) &data, 1);
    if (data == 0x35){
        /* setup 3 as data size of DataFrame::FRAME_TYPE_DATA */
        target->setSize(3);
    }
    else if (data == 0x36){
        /* setup 3 as data size of DataFrame::FRAME_TYPE_DATA */
        target->setSize(2);
    }
    else {
        /* invalid value found -> trigger stop flag to __readFramedData__ method */
        obj->trigInvDataIndicator();
    }
}

void serialSetupDataFrameProtocol(Serialink &serial){
    /* Frame Data Format / Protocol Example
     * | Start  Bytes |   Command   |  Main Data   | CRC16 Validator | Stop Bytes |
     * |:-------------|:------------|:-------------|:----------------|:-----------|
     * |    4 bytes   |   1 byte    |   N bytes    |     2 bytes     | 4 bytes    |
     * |  0x31323334  | 0x35 / 0x36 | based on Cmd |  init = 0x0000  | 0x39302D3D |
     *
     * If Command = 0x35, then the Main Data length is 3 bytes.
     * If Command = 0x36, then the Main Data length is 2 bytes.
     * If Command is not equal to 0x35 and 0x36, then the data is invalid.
     *
     * Use this input example: 3132333435363738159039302D3D
     */
    /* Configure Frame Format */
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    /* Setup the handler function to determine the data length of DataFrame::FRAME_TYPE_DATA.
     * This function is called after all data from DataFrame::FRAME_TYPE_COMMAND is received.
     */
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, (void *) &serial);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame crcValidatorBytes(DataFrame::FRAME_TYPE_VALIDATOR, 2);
    /* Setup the handler function to validate Data by using crc16 validation.
     * This function is called after all data from DataFrame::FRAME_TYPE_VALIDATOR is received.
     */
    crcValidatorBytes.setPostExecuteFunction((const void *) &crc16, (void *) &serial);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    /* Setup Frame Format to serial com */
    serial = startBytes + cmdBytes + dataBytes + crcValidatorBytes + stopBytes;
}

int main(int argc, char **argv){
    if (argc != 4){
        std::cout << "cmd: " << argv[0] << " <port> <timeout100ms> <keepAliveMs>" << std::endl;
        exit(0);
    }
    int ret = 0;
    std::vector <unsigned char> data;
    Serialink serial;
    /* Prepare Object */
    serial.setPort(argv[1]);
    serial.setBaudrate(B115200);
    serial.setTimeout(atoi(argv[2]));
    serial.setKeepAlive(atoi(argv[3]));
    serialSetupDataFrameProtocol(serial);
    /* Do serial communication */
    serial.openPort();
    while ((ret = serial.readFramedData()) != 0){
        if (ret == 4){
            std::cout << "Invalid Received Data Details:" << std::endl;
            if (serial.getBuffer(data) > 0){
                std::cout << "    Received Data: ";
                displayData(data);
            }
            if (serial.getRemainingBuffer(data) > 0){
                std::cout << "    Remaining Received Data: ";
                displayData(data);
            }
            std::cout << std::endl;
        }
        else {
            std::cout << errorMessage[ret - 1] << std::endl;
        }
    }
    serial.closePort();
    serial.getFormat()->getAllData(data);
    std::cout << "Received Success [" + std::to_string(serial.getDataSize()) + "]" << std::endl;
    std::cout << "    Data: ";
    displayData(data);
    return 0;
}
```

Save file by press `[Ctrl + o]` on your keyboard and then press `[Enter]`. After that press `[Ctrl + x]`.

4. Create a `CMakeLists.txt` file.

```bash
nano CMakeLists.txt
```

Here is `cmake` configuration:

```bash
cmake_minimum_required(VERSION 3.0.0)
set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "1")
set(PROJECT_VERSION_PATCH "0")
project(SerialReceiver VERSION 1.1.0 LANGUAGES CXX)

include(FindPkgConfig)

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Find and Check Library
find_package(PkgConfig REQUIRED)

# Specify the source files
set(SOURCE_FILES
    src/main.cpp
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/Serialink/include>
  $<INSTALL_INTERFACE:include>
)

# Ensure Serialink is built before the executable
add_dependencies(${PROJECT_NAME} Serialink-lib)

# Link the executable with the library
target_link_libraries(${PROJECT_NAME} PRIVATE Serialink-lib DataFrame-lib)
target_link_libraries(${PROJECT_NAME} PUBLIC -lpthread -lusb-1.0)

add_subdirectory(external/Serialink EXCLUDE_FROM_ALL)
set(USE_EXE_FUNC ON)
set(USE_POST_EXE_FUNC ON)
add_definitions(-D__USE_EXE_FUNC)
add_definitions(-D__USE_POST_FUNC)

# Set compiler and linker flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -O3")
set(CMAKE_USE_RELATIVE_PATHS OFF)
```

Save file by press `[Ctrl + o]` on your keyboard and then press `[Enter]`. After that press `[Ctrl + x]`.

### Explanation

- Line 29: The `$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/Serialink/include>` command adds a reference to the header file location of the Serialink Library. This ensures that during the build process, CMake can locate the Serialink headers correctly.

- Line 34: The `add_dependencies(${PROJECT_NAME} Serialink-lib)` command specifies that the Serialink Library must be built before the main application. This ensures that the Serialink Library is compiled before the main project depends on it.

- Line 37: The `target_link_libraries(${PROJECT_NAME} PRIVATE Serialink-lib DataFrame-lib)` command links the Serialink Library and the DataFrame Library (a dependency of the Serialink Library) to the main application as private libraries. This means that these libraries are used internally by the main project.

- Line 40: The `add_subdirectory(external/Serialink EXCLUDE_FROM_ALL)` command adds the external/Serialink directory as part of the main project. The EXCLUDE_FROM_ALL option prevents CMake from building this subdirectory by default unless explicitly requested.

5. Build the project.

```bash
mkdir build
cd build
cmake ..
make
```

6. Run the application.

```bash
./SerialReceiver <port> <timeout100ms> <keepAliveMs>
```

Example:

```bash
./SerialReceiver /dev/ttyUSB0 10 500
```
