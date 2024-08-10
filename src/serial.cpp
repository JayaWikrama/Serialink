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
 * @brief setter untuk file descriptor.
 *
 * Berfungsi untuk melakukan setup nilai file descriptor.
 * @param fd file descriptor.
 */
#if defined(PLATFORM_POSIX) || defined(__linux__)
void Serial::setFileDescriptor(int fd)
#else
void Serial::setFileDescriptor(HANDLE fd)
#endif
{
    pthread_mutex_lock(&(this->mtx));
    this->fd = fd;
    pthread_mutex_unlock(&(this->mtx));
}

/**
 * @brief getter untuk file descriptor.
 *
 * Berfungsi untuk melakukan pengambilan informasi nilai file descriptor.
 * @return file descriptor.
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
 * @brief setup serial attributes.
 *
 * Berfungsi untuk melakukan setup atau pengaturan pada atribut file descriptor dari port serial yang sukses terbuka.
 * @return true jika sukses
 * @return false jika gagal
 */
bool Serial::setupAttributes(){
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
    return result;
}

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
    this->keepAliveMs = 0;
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
    this->keepAliveMs = 0;
    this->port = port;
    pthread_mutex_init(&(this->mtx), NULL);
}

/**
 * @brief Custom constructor.
 *
 * Berfungsi untuk melakukan setup private data dan parameter ke nilai default (kecuali untuk port, baud, timeout, dan keepAliveMs). Diantaranya:
 * fd = -1
 * Initialize mutex
 * @param port port device serial.
 * @param baud baudrate.
 * @param timeout timeout per 100ms.
 * @param keepAliveMs waktu dalam milliseconds.
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
}

/**
 * @brief Destructor.
 *
 * Berfungsi untuk melakukan release setiap memory yang dialokasikan.
 */
Serial::~Serial(){
    pthread_mutex_lock(&(this->mtx));
#if defined(PLATFORM_POSIX) || defined(__linux__)
    if (this->fd > 0){
        close(this->fd);
        this->fd = -1;
    }
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
 * @brief getter untuk port serial.
 *
 * Berfungsi untuk melakukan pengambilan data port serial yang digunakan
 * @return string port.
 */
std::string Serial::getPort(){
    return this->port;
}

/**
 * @brief getter untuk baudrate.
 *
 * Berfungsi untuk melakukan pengambilan data baudrate komunikasi serial yang digunakan
 * @return baudrate.
 */
speed_t Serial::getBaudrate(){
    return this->baud;
}

/**
 * @brief getter untuk timeout komunikasi.
 *
 * Berfungsi untuk melakukan pengambilan data timeout komunikasi serial
 * @return timeout per 100ms.
 */
unsigned int Serial::getTimeout(){
    return this->timeout;
}

/**
 * @brief getter untuk keep alive komunikasi.
 *
 * Berfungsi untuk melakukan pengambilan data maksimal waktu tunggu untuk membaca data serial selanjutnya setelah bytes pertama data serial berhasil diterima
 * @return timeout per 100ms.
 */
unsigned int Serial::getKeepAlive(){
    return this->keepAliveMs;
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
    pthread_mutex_unlock(&(this->mtx));
    bool success = this->setupAttributes();
    pthread_mutex_lock(&(this->mtx));
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
 * Berfungsi untuk melakukan operasi pembacaan data serial tanpa memisahkan data yang sukses terbaca menjadi data dengan size yang diinginkan dengan data sisa. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param sz jumlah data yang ingin dibaca. __sz__ = 0 berarti jumlah data yang akan dibaca tidak terbatas (hingga __keepAliveMs__ terpenuhi).
 * @param dontSplitRemainingData mode untuk melakukan penonaktifkan fungsi pemisahan data secara otomatis berdasarkan jumlah data yang ingin dibaca.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readData(size_t sz, bool dontSplitRemainingData){
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
 * @brief berfungsi untuk melakukan operasi pembacaan data serial.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param sz jumlah data yang ingin dibaca. __sz__ = 0 berarti jumlah data yang akan dibaca tidak terbatas (hingga __keepAliveMs__ terpenuhi).
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readData(size_t sz){
    return this->readData(sz, false);
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readData(){
    return this->readData(0, false);
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param startBytes data start bytes yang ingin ditemukan.
 * @param sz ukuran data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
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
    return ret;
}

/**
 * @brief function overloading untuk __readStartBytes__ dengan input menggunakan const char *.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param startBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readStartBytes(const char *startBytes){
    return this->readStartBytes((const unsigned char *) startBytes, strlen(startBytes));
}

/**
 * @brief function overloading untuk __readStartBytes__ dengan input menggunakan vector.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param startBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readStartBytes(const std::vector <unsigned char> startBytes){
    return this->readStartBytes(startBytes.data(), startBytes.size());
}

/**
 * @brief function overloading untuk __readStartBytes__ dengan input menggunakan string.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param startBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readStartBytes(const std::string startBytes){
    return this->readStartBytes((const unsigned char *) startBytes.c_str(), startBytes.length());
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @param sz ukuran data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
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
 * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan const char *.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readUntilStopBytes(const char *stopBytes){
    return this->readUntilStopBytes((const unsigned char *) stopBytes, strlen(stopBytes));
}

/**
 * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan vector.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readUntilStopBytes(const std::vector <unsigned char> stopBytes){
    return this->readUntilStopBytes(stopBytes.data(), stopBytes.size());
}

/**
 * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan string.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 */
int Serial::readUntilStopBytes(const std::string stopBytes){
    return this->readUntilStopBytes((const unsigned char *) stopBytes.c_str(), stopBytes.length());
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial sekaligus pengecekan apakah data tersebut adalah stop bytes yang diinginkan.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial sekaligus melakukan pengecekan apakah data tersebut adalah stop bytes yang diinginkan. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @param sz ukuran data start bytes yang ingin ditemukan.
 * @return 0 jika sukses dan data valid.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika terdapat data serial yang terbaca tetapi tidak sesuai dengan stop bytes yang diinginkan.
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
 * @brief function overloading untuk __readStopBytes__ dengan input menggunakan const char *.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial sekaligus melakukan pengecekan apakah data tersebut adalah stop bytes yang diinginkan. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses dan data valid.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika terdapat data serial yang terbaca tetapi tidak sesuai dengan stop bytes yang diinginkan.
 */
int Serial::readStopBytes(const char *stopBytes){
    return this->readStopBytes((const unsigned char *) stopBytes, strlen(stopBytes));
}

/**
 * @brief function overloading untuk __readStopBytes__ dengan input menggunakan vector.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial sekaligus melakukan pengecekan apakah data tersebut adalah stop bytes yang diinginkan. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses dan data valid.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika terdapat data serial yang terbaca tetapi tidak sesuai dengan stop bytes yang diinginkan.
 */
int Serial::readStopBytes(const std::vector <unsigned char> stopBytes){
    return this->readStopBytes(stopBytes.data(), stopBytes.size());
}

/**
 * @brief function overloading untuk __readStopBytes__ dengan input menggunakan string.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial sekaligus melakukan pengecekan apakah data tersebut adalah stop bytes yang diinginkan. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param stopBytes data start bytes yang ingin ditemukan.
 * @return 0 jika sukses dan data valid.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
 * @return 3 jika terdapat data serial yang terbaca tetapi tidak sesuai dengan stop bytes yang diinginkan.
 */
int Serial::readStopBytes(const std::string stopBytes){
    return this->readStopBytes((const unsigned char *) stopBytes.c_str(), stopBytes.length());
}

/**
 * @brief berfungsi untuk melakukan operasi pembacaan data serial hingga sejumlah data yang diinginkan terpenuhi.
 *
 * Berfungsi untuk melakukan operasi pembacaan data serial hingga sejumlah data yang diinginkan terpenuhi. Pengulangan dilakukan maksimal 3 kali terhitung setelah data pertama diterima. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
 * @param sz ukuran data serial yang ingin dibaca.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika timeout.
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
 * @brief berfungsi untuk melakukan pengambilan jumlah data yang berhasil terbaca.
 *
 * Berfungsi untuk mengambil informasi nilai jumlah data yang berhasil terbaca.
 * @return ukuran atau size data serial dalam bytes.
 */
size_t Serial::getDataSize(){
    return this->data.size();
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
 * @brief berfungsi untuk melakukan pengambilan data buffer read.
 *
 * Berfungsi untuk mengambil semua data yang telah sukses terbaca pada method read.
 * @param buffer variable untuk menampung data serial yang sukses terbaca.
 * @return ukuran atau size data serial.
 */
size_t Serial::getBuffer(std::vector <unsigned char> &buffer){
    buffer.clear();
    buffer.assign(this->data.begin(), this->data.end());
    return buffer.size();
}

/**
 * @brief berfungsi untuk melakukan pengambilan jumlah data yang terdapat pada remaining buffer.
 *
 * Berfungsi untuk mengambil informasi nilai jumlah data yang terdapat pada remaining buffer.
 * @return ukuran atau size data serial dalam bytes.
 */
size_t Serial::getRemainingDataSize(){
    return this->remainingData.size();
}

/**
 * @brief berfungsi untuk melakukan pengambilan sisa data serial yang sukses terbaca diluar data buffer.
 *
 * Berfungsi untuk mengambil semua sisa data serial yang telah sukses terbaca diluar data buffer.
 * @param buffer variable untuk menampung data serial yang sukses terbaca.
 * @param maxBufferSz batasan ukuran data maksimum yang dapat ditampung oleh variable buffer.
 * @return ukuran atau size data serial.
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
 * @brief function overloading untuk __readStopBytes__ dengan parameter output menggunakan vector.
 *
 * Berfungsi untuk mengambil semua sisa data serial yang telah sukses terbaca diluar data buffer.
 * @param buffer variable untuk menampung data serial yang sukses terbaca.
 * @return ukuran atau size data serial.
 */
size_t Serial::getRemainingBuffer(std::vector <unsigned char> &buffer){
    buffer.clear();
    buffer.assign(this->remainingData.begin(), this->remainingData.end());
    return buffer.size();
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
 * @brief berfungsi untuk melakukan operasi penulisan data serial.
 *
 * Berfungsi untuk melakukan operasi penulisan data serial.
 * @param buffer data yang ingin ditulis.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika gagal melakukan penulisan data.
 */
int Serial::writeData(const char *buffer){
    return this->writeData((const unsigned char *) buffer, strlen(buffer));
}

/**
 * @brief berfungsi untuk melakukan operasi penulisan data serial.
 *
 * Berfungsi untuk melakukan operasi penulisan data serial.
 * @param buffer data yang ingin ditulis.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika gagal melakukan penulisan data.
 */
int Serial::writeData(const std::vector <unsigned char> buffer){
    return this->writeData(buffer.data(), buffer.size());
}

/**
 * @brief berfungsi untuk melakukan operasi penulisan data serial.
 *
 * Berfungsi untuk melakukan operasi penulisan data serial.
 * @param buffer data yang ingin ditulis.
 * @return 0 jika sukses.
 * @return 1 jika port belum terbuka.
 * @return 2 jika gagal melakukan penulisan data.
 */
int Serial::writeData(const std::string buffer){
    return this->writeData((const unsigned char *) buffer.c_str(), buffer.length());
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
