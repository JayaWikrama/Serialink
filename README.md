
# Library Serialink

Serialink adalah sebuah library untuk komunikasi serial yang dirancang khusus untuk mendukung dan mempermudah proses pengembangan perangkat lunak pada lingkungan Linux. Library ini menyediakan antarmuka (interface) dan pengaturan yang sederhana, sehingga mudah diterapkan dalam berbagai aplikasi. Selain itu, Serialink mendukung komunikasi serial yang lebih advance, termasuk komunikasi yang menggunakan protokol khusus (framed data).

## Persiapan Installasi Library

Lakukan installasi paket-paket berikut.

### Installasi Compiller dan CMake

1. Installasi Compiler:

```bash
sudo apt install build-essential g++ binutils
```

3. Installasi CMake:

```bash
sudo apt install make cmake
```

### Installasi Google Test

1. Clone repository:

```bash
git clone https://github.com/google/googletest.git -b v1.14.0
```

2. Masuk ke directory dari repository yang sudah di-clone:

```bash
cd googletest
```

3. Compile Google Test Library:

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

5. Kembali ke root directory:

```bash
cd ../../
```

## Compile Library

1. Clone main repository:

```bash
git clone https://github.com/JayaWikrama/Serialink.git
```

2. Pindah ke direktori proyek:

```bash
cd Serialink
```

3. Clone semua git submodule:

```bash
git submodule init 
git submodule update --init --recursive
```

4. Jika suatu saat ingin menggunakan source code yang terbaru (yang sama dengan remote repository), jalankan perintah berikut:

```bash
git submodule update --remote
```

5. Buat direktori build:

```bash
mkdir build
cd build
```

6. Konfigurasi CMake:

```bash
cmake ..
```

7. Lakukan kompilasi:

```bash
make
```

8. Jalankan unit test untuk memastikan semua fungsi berjalan dengan normal:

```bash
./Serialink-test
```

Output hasil test:

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

## Penggunaan Library

Salah satu cara menggunakan library ini adalah dengan mengintegrasikannya ke aplikasi utama sebagai git submodule. Berikut contoh cara membuat project baru dan mengintegrasikan library Serialink kedalamnya:

1. Buat project tree-nya terlebih dahulu.

```bash
mkdir SerialReceiveApp
cd SerialReceiveApp
mkdir src
mkdir include
mkdir external
git init .
```

2. Tambahkan library Serialink sebagai external library

```bash
git submodule add https://github.com/JayaWikrama/Serialink.git external/Serialink
git submodule update --init --recursive
```

3. Buat source code aplikasi utama.

```bash
nano src/main.cpp
```

Berikut source code aplikasinya.

```cpp
#include <iostream>
#include <string.h>

#include "serialink.hpp"

const std::string errorMessage[] = {
    "Serial Port Has Not Been Opened",
    "Timeout",
    "Frame Format Has Not Been Setup"
};

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
     * | Start  Bytes |   Command   |  Main Data   | Stop Bytes |
     * |:-------------|:------------|:-------------|:-----------|
     * |    4 bytes   |   1 byte    |   N bytes    | 4 bytes    |
     * |  0x31323334  | 0x35 / 0x36 | based on Cmd | 0x39302D3D |
     *
     * If Command = 0x35, then the Main Data length is 3 bytes.
     * If Command = 0x36, then the Main Data length is 2 bytes.
     * If Command is not equal to 0x35 and 0x36, then the data is invalid.
     */
    /* Configure Frame Format */
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    /* Setup the handler function to determine the data length of DataFrame::FRAME_TYPE_DATA.
     * This function is called after all data from DataFrame::FRAME_TYPE_COMMAND is received.
     */
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, (void *) &serial);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    /* Setup Frame Format to serial com */
    serial = startBytes + cmdBytes + dataBytes + stopBytes;
}

void displayData(const std::vector <unsigned char> &data){
    for (auto i = data.begin(); i < data.end(); ++i){
        std::cout << std::hex << (int) *i;
        std::cout << " ";
    }
    std::cout << std::endl;
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

Simpan file dengan menekan `[Ctrl + o]` kemudian tekan `[Enter]`. Selanjutnya keluar dari editor dengan menekan `[Ctrl + x]`.

4. Buat konfigurasi `cmake` untuk project tersebut.

```bash
nano CMakeLists.txt
```

Ketik konfigurasi berikut:

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
target_link_libraries(${PROJECT_NAME} PUBLIC -lpthread)

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

Simpan file dengan menekan `[Ctrl + o]` kemudian tekan `[Enter]`. Selanjutnya keluar dari editor dengan menekan `[Ctrl + x]`.

### Penjelasan

- Pada line 29, terdapat `$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/external/Serialink/include>` yang berfungsi untuk menambah referensi lokasi file header dari Serialink Library.

- Pada line 34, terdapat `add_dependencies(${PROJECT_NAME} Serialink-lib)` yang berfungsi untuk menjadikan Serialink Library sebagai dependency dari aplikasi utama. Sehingga library tersebut akan dibuat/di-compile terlebih dahulu sebelum aplikasi utama.

- Pada line 37, terdapat `target_link_libraries(${PROJECT_NAME} PRIVATE Serialink-lib DataFrame-lib)` yang berfungsi untuk menghubungkan Serialink Library dan DataFrame Library (dependency Serialink Library) ke aplikasi utama sebagai Private Library.

- Pada line 40, terdapat `add_subdirectory(external/Serialink EXCLUDE_FROM_ALL)` yang berfungsi untuk menambahkan directory `external/Serialink` sebagai bagian dari project utama.

5. Project sudah siap dikompilasi. Lakukan kompilasi dengan perintah berikut:

```bash
mkdir build
cd build
cmake ..
make
```

6. Setelah project berhasil terkompilasi, aplikasi dapat dijalankan dengan perintah berikut:

```bash
./SerialReceiver <port> <timeout100ms> <keepAliveMs>
```

Contoh:

```bash
./SerialReceiver /dev/ttyUSB0 10 500
```

yang berarti aplikasi `SerialReceiver` akan melakukan pembacaan terhadap port `/dev/ttyUSB0` dengan timeout setiap pembacaan sebesar `1 detik` dan timeout antar bytes (setelah byte pertama diterima) sebesar `500ms`.
