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
 * @brief Default constructor.
 *
 * Berfungsi untuk melakukan setup private data dan parameter ke nilai default. Diantaranya:
 * fd = -1
 * baud = B9600
 * timeout = 10 (1 detik)
 * keepAliveMs = 0
 * port = /dev/ttyUSB0
 * Initialize mutex
 */
Serial::Serial(){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = B9600;
    this->timeout = 10;
    this->port = "/dev/ttyUSB0";
    pthread_mutex_init(&(this->mtx), NULL);
}

/**
 * @brief Custom constructor.
 *
 * Berfungsi untuk melakukan setup private data dan parameter ke nilai default (kecuali untuk port, baud, dan timeout). Diantaranya:
 * fd = -1
 * keepAliveMs = 0
 * Initialize mutex
 * @param port port device serial.
 * @param baud baudrate.
 * @param timeout timeout per 100ms.
 */
Serial::Serial(const std::string port, speed_t baud, unsigned int timeout){
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = -1;
#endif
    this->baud = baud;
    this->timeout = timeout;
    this->port = port;
    pthread_mutex_init(&(this->mtx), NULL);
}

/**
 * @brief Destructor.
 *
 * Berfungsi untuk melakukan release setiap memory yang dialokasikan.
 */
Serial::~Serial(){
    pthread_mutex_lock(&(this->mtx));
    #if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd > 0) close(this->fd);
    this->fd = -1;
#else
    CloseHandle(this->fd);
#endif
    pthread_mutex_unlock(&(this->mtx));
    pthread_mutex_destroy(&(this->mtx));
}

/**
 * @brief setter untuk port device serial.
 *
 * Berfungsi untuk melakukan setup port device serial yang digunakan
 * @param port port device serial.
 */
void Serial::setPort(const std::string port){
    pthread_mutex_lock(&(this->mtx));
    this->port = port;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief setter untuk port baudrate.
 *
 * Berfungsi untuk melakukan setup baudrate komunikasi yang digunakan
 * @param baud baudrate.
 */
void Serial::setBaudrate(speed_t baud){
    pthread_mutex_lock(&(this->mtx));
    this->baud = baud;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief setter untuk timeout komunikasi.
 *
 * Berfungsi untuk melakukan setup timeout komunikasi serial
 * @param timeout timeout per 100ms.
 */
void Serial::setTimeout(unsigned int timeout){
    pthread_mutex_lock(&(this->mtx));
    this->timeout = timeout;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief setter untuk keep alive komunikasi.
 *
 * Berfungsi untuk melakukan setup maksimal waktu tunggu untuk membaca data serial selanjutnya setelah bytes pertama data serial berhasil diterima.
 * @param keepAliveMs waktu dalam Milliseconds.
 */
void Serial::setKeepAlive(unsigned int keepAliveMs){
    pthread_mutex_lock(&(this->mtx));
    this->keepAliveMs = keepAliveMs;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief open port serial.
 *
 * Berfungsi untuk melakukan open port serial komunikasi
 * @return 0 jika sukses
 * @return 1 jika gagal
 */
int Serial::openPort(){
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    this->fd = open (this->port.c_str(), O_RDWR | O_NOCTTY | O_SYNC);
	if (this->fd <= 0)
#else
    this->fd = CreateFileA(this->port.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (this->fd == INVALID_HANDLE_VALUE)
#endif
    {
        pthread_mutex_unlock(&(this->mtx));
		return 1;
	}
    bool success = false;
#if defined(PLATFORM_POSIX) || defined(__linux__)
    struct termios ttyAttr;
    memset (&ttyAttr, 0, sizeof(ttyAttr));
    success = (tcgetattr(this->fd, &ttyAttr) == 0);
#else
    success = FlushFileBuffers(this->fd);
#endif
    if (success == false){
        pthread_mutex_unlock(&(this->mtx));
        return 1;
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
    success = (tcsetattr (this->fd, TCSANOW, &ttyAttr) == 0);
#else
    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = this->timeout;
    timeouts.ReadTotalTimeoutMultiplier = 50;
    timeouts.WriteTotalTimeoutConstant = this->timeout;
    timeouts.WriteTotalTimeoutMultiplier = 50;
    success = SetCommTimeouts(this->fd, &timeouts);
    if (success == true){
        DCB state = {0};
        state.DCBlength=sizeof(DCB);
        success = GetCommState(this->fd, &state);
        if (success == true){
            state.BaudRate = static_cast<uint32_t>(this->baud);
            state.ByteSize = 8;
            state.Parity = NOPARITY;
            state.StopBits = ONESTOPBIT;
            success = SetCommState(this->fd, &state);
        }
    }
#endif
    if (success == false){
#if defined(PLATFORM_POSIX) || defined(__linux__)
        close(this->fd);
        this->fd = -1;
#else
        CloseHandle(this->fd);
#endif
        pthread_mutex_unlock(&(this->mtx));
        return 1;
    }
    pthread_mutex_unlock(&(this->mtx));
    return 0;
}

#if defined(PLATFORM_POSIX) || defined(__linux__)
/**
 * @brief berfungsi untuk melakukan penyecekan input bytes.
 *
 * Berfungsi untuk melakukan pengecekan apakah ada input bytes pada buffer serial descriptor.
 * @return true jika terdapat input bytes pada buffer serial descriptor.
 * @return false jika tidak terdapat input bytes pada buffer serial descriptor.
 */
bool Serial::isInputBytesAvailable(){
    pthread_mutex_lock(&(this->mtx));
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
 * @brief berfungsi untuk melakukan operasi pembacaan data serial.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readData(){
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd <= 0){
        pthread_mutex_unlock(&(this->mtx));
        return 1;
    }
    ssize_t bytes = 0;
#else
    long unsigned int bytes = 0;
#endif
    int idx = 0;
    unsigned char tmp[128];
    this->data.clear();
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
    	bytes = read(this->fd, (void *) tmp, sizeof(tmp));
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
    } while (bytes > 0);
    if (this->data.size() == 0){
        pthread_mutex_unlock(&(this->mtx));
        return 2;
    }
    pthread_mutex_unlock(&(this->mtx));
    return 0;
}

/**
 * @brief berfungsi untuk melakukan pengambilan data buffer read.
 *
 * Berfungsi untuk mengambil semua data yang telah sukses terbaca pada method read.
 * @param buffer variable untuk menampung data serial yang sukses terbaca.
 * @param maxBufferSz batasan ukuran data maksimum yang dapat ditampung oleh variable buffer.
 * @return ukuran atau size data serial.
 */
size_t Serial::getBuffer(unsigned char *buffer, size_t maxBufferSz){
    pthread_mutex_lock(&(this->mtx));
    size_t result = (this->data.size() < maxBufferSz ? this->data.size() : maxBufferSz);
    for (auto i = this->data.begin(); i != this->data.end(); i++){
        if (result == 0) break;
        *buffer = *i;
        buffer++;
        result--;
    }
    pthread_mutex_unlock(&(this->mtx));
    return result;
}

/**
 * @brief berfungsi untuk melakukan operasi penulisan data serial.
 *
 * Berfungsi untuk melakukan operasi penulisan data serial.
 * @param buffer data yang ingin ditulis.
 * @param sz ukuran data yang ingin ditulis.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika gagal melakukan penulisan data.
 */
int Serial::writeData(const unsigned char *buffer, size_t sz){
    pthread_mutex_lock(&(this->mtx));
    size_t total = 0;
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd <= 0){
        pthread_mutex_unlock(&(this->mtx));
        return 1;
    }
    ssize_t bytes = 0;
#else
    long unsigned int bytes = 0;
#endif
    while (total < sz){
#if defined(PLATFORM_POSIX) || defined(__linux__)
        bytes = write(this->fd, (void *) (buffer + total), sz - total);
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
            pthread_mutex_unlock(&(this->mtx));
            return 2;
        }
    }
    pthread_mutex_unlock(&(this->mtx));
    return 0;
}

/**
 * @brief menutup port serial komunikasi.
 *
 * Berfungsi untuk melakukan penutupan port serial komunikasi
 */
void Serial::closePort(){
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd > 0) close(this->fd);
    this->fd = -1;
#else
    CloseHandle(this->fd);
#endif
    pthread_mutex_unlock(&(this->mtx));
}