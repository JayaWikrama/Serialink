#ifndef __SERIAL_BASIC_HPP__
#define __SERIAL_BASIC_HPP__

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
/* Untuk Windows saya buat tanpa Mutex dan thread */
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
#include "data-frame.hpp"

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
    std::vector <unsigned char> data;
    std::vector <unsigned char> remainingData;
    std::string port;
    pthread_mutex_t mtx;
  public:
    DataFrame *frameFormat;
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
    Serial();

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
    Serial(const std::string port, speed_t baud, unsigned int timeout);

    /**
     * @brief Destructor.
     *
     * Berfungsi untuk melakukan release setiap memory yang dialokasikan.
     */
    ~Serial();

    /**
     * @brief setter untuk port device serial.
     *
     * Berfungsi untuk melakukan setup port device serial yang digunakan
     * @param port port device serial.

     */
    void setPort(const std::string port);

    /**
     * @brief setter untuk port baudrate.
     *
     * Berfungsi untuk melakukan setup baudrate komunikasi yang digunakan
     * @param baud baudrate.
     */
    void setBaudrate(speed_t baud);

    /**
     * @brief setter untuk timeout komunikasi.
     *
     * Berfungsi untuk melakukan setup timeout komunikasi serial
     * @param timeout timeout per 100ms.
     */
    void setTimeout(unsigned int timeout);

    /**
     * @brief setter untuk keep alive komunikasi.
     *
     * Berfungsi untuk melakukan setup maksimal waktu tunggu untuk membaca data serial selanjutnya setelah bytes pertama data serial berhasil diterima.
     * @param keepAliveMs waktu dalam Milliseconds.
     */
    void setKeepAlive(unsigned int keepAliveMs);

    /**
     * @brief getter untuk port serial.
     *
     * Berfungsi untuk melakukan pengambilan data port serial yang digunakan
     * @return string port.
     */
    std::string getPort();
    
    /**
     * @brief getter untuk baudrate.
     *
     * Berfungsi untuk melakukan pengambilan data baudrate komunikasi serial yang digunakan
     * @return baudrate.
     */
    speed_t getBaudrate();

    /**
     * @brief getter untuk timeout komunikasi.
     *
     * Berfungsi untuk melakukan pengambilan data timeout komunikasi serial
     * @return timeout per 100ms.
     */
    unsigned int getTimeout();

    /**
     * @brief open port serial.
     *
     * Berfungsi untuk melakukan open port serial komunikasi
     * @return 0 jika sukses
     * @return 1 jika gagal
     */
    int openPort();

#if defined(PLATFORM_POSIX) || defined(__linux__)
    /**
     * @brief berfungsi untuk melakukan penyecekan input bytes.
     *
     * Berfungsi untuk melakukan pengecekan apakah ada input bytes pada buffer serial descriptor.
     * @return true jika terdapat input bytes pada buffer serial descriptor.
     * @return false jika tidak terdapat input bytes pada buffer serial descriptor.
     */
    bool isInputBytesAvailable();
#endif

    /**
     * @brief berfungsi untuk melakukan operasi pembacaan data serial.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readData();

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
    int readStartBytes(const unsigned char *startBytes, size_t sz);

    /**
     * @brief berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readFramedData();

    /**
     * @brief berfungsi untuk melakukan pengambilan data buffer read.
     *
     * Berfungsi untuk mengambil semua data yang telah sukses terbaca pada method read.
     * @param buffer variable untuk menampung data serial yang sukses terbaca.
     * @param maxBufferSz batasan ukuran data maksimum yang dapat ditampung oleh variable buffer.
     * @return ukuran atau size data serial.
     */
    size_t getBuffer(unsigned char *buffer, size_t maxBufferSz);

    /**
     * @brief berfungsi untuk melakukan pengambilan sisa data serial yang sukses terbaca diluar data buffer.
     *
     * Berfungsi untuk mengambil semua sisa data serial yang telah sukses terbaca diluar data buffer.
     * @param buffer variable untuk menampung data serial yang sukses terbaca.
     * @param maxBufferSz batasan ukuran data maksimum yang dapat ditampung oleh variable buffer.
     * @return ukuran atau size data serial.
     */
    size_t getRemainingBuffer(unsigned char *buffer, size_t maxBufferSz);

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
    int writeData(const unsigned char *buffer, size_t sz);

    /**
     * @brief menutup port serial komunikasi.
     *
     * Berfungsi untuk melakukan penutupan port serial komunikasi
     */
    void closePort();
};

#endif