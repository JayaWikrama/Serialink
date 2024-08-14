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
  protected:
    std::vector <unsigned char> data;
    std::vector <unsigned char> remainingData;
    /**
     * @brief setter untuk file descriptor.
     *
     * Berfungsi untuk melakukan setup nilai file descriptor.
     * @param fd file descriptor.
     */
#if defined(PLATFORM_POSIX) || defined(__linux__)
    void setFileDescriptor(int fd);
#else
    void setFileDescriptor(HANDLE fd);
#endif

    /**
     * @brief getter untuk file descriptor.
     *
     * Berfungsi untuk melakukan pengambilan informasi nilai file descriptor.
     * @return file descriptor.
     */
#if defined(PLATFORM_POSIX) || defined(__linux__)
    int getFileDescriptor();
#else
    HANDLE getFileDescriptor();
#endif

    /**
     * @brief setup serial attributes.
     *
     * Berfungsi untuk melakukan setup atau pengaturan pada atribut file descriptor dari port serial yang sukses terbuka.
     * @return true jika sukses
     * @return false jika gagal
     */
    bool setupAttributes();
  public:
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
    Serial(const std::string port, speed_t baud, unsigned int timeout, unsigned int keepAliveMs);

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
     * @brief getter untuk keep alive komunikasi.
     *
     * Berfungsi untuk melakukan pengambilan data maksimal waktu tunggu untuk membaca data serial selanjutnya setelah bytes pertama data serial berhasil diterima
     * @return timeout per 100ms.
     */
    unsigned int getKeepAlive();

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
     * Berfungsi untuk melakukan operasi pembacaan data serial tanpa memisahkan data yang sukses terbaca menjadi data dengan size yang diinginkan dengan data sisa. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param sz jumlah data yang ingin dibaca. __sz__ = 0 berarti jumlah data yang akan dibaca tidak terbatas (hingga __keepAliveMs__ terpenuhi).
     * @param dontSplitRemainingData mode untuk melakukan penonaktifkan fungsi pemisahan data secara otomatis berdasarkan jumlah data yang ingin dibaca.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readData(size_t sz, bool dontSplitRemainingData);

    /**
     * @brief berfungsi untuk melakukan operasi pembacaan data serial.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param sz jumlah data yang ingin dibaca. __sz__ = 0 berarti jumlah data yang akan dibaca tidak terbatas (hingga __keepAliveMs__ terpenuhi).
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readData(size_t sz);

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
     * @brief function overloading untuk __readStartBytes__ dengan input menggunakan const char *.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param startBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readStartBytes(const char *startBytes);

    /**
     * @brief function overloading untuk __readStartBytes__ dengan input menggunakan vector.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param startBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readStartBytes(const std::vector <unsigned char> startBytes);

    /**
     * @brief function overloading untuk __readStartBytes__ dengan input menggunakan string.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya start bytes yang diinginkan. Data serial sebelum start bytes yang diinginkan secara otomatis dihapus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param startBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readStartBytes(const std::string startBytes);

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
    int readUntilStopBytes(const unsigned char *stopBytes, size_t sz);

    /**
     * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan const char *.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param stopBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readUntilStopBytes(const char *stopBytes);

    /**
     * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan vector.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param stopBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readUntilStopBytes(const std::vector <unsigned char> stopBytes);

    /**
     * @brief function overloading untuk __readUntilStopBytes__ dengan input menggunakan string.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga ditemukannya stop bytes yang diinginkan. Data serial sebelum stop bytes yang diinginkan secara otomatis ikut tersimpan kedalam buffer. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param stopBytes data start bytes yang ingin ditemukan.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readUntilStopBytes(const std::string stopBytes);

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
    int readStopBytes(const unsigned char *stopBytes, size_t sz);

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
    int readStopBytes(const char *stopBytes);

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
    int readStopBytes(const std::vector <unsigned char> stopBytes);

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
    int readStopBytes(const std::string stopBytes);

    /**
     * @brief berfungsi untuk melakukan operasi pembacaan data serial hingga sejumlah data yang diinginkan terpenuhi.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial hingga sejumlah data yang diinginkan terpenuhi. Pengulangan dilakukan maksimal 3 kali terhitung setelah data pertama diterima. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @param sz ukuran data serial yang ingin dibaca.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     */
    int readNBytes(size_t sz);

    /**
     * @brief berfungsi untuk melakukan pengambilan jumlah data yang berhasil terbaca.
     *
     * Berfungsi untuk mengambil informasi nilai jumlah data yang berhasil terbaca.
     * @return ukuran atau size data serial dalam bytes.
     */
    size_t getDataSize();

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
     * @brief berfungsi untuk melakukan pengambilan data buffer read.
     *
     * Berfungsi untuk mengambil semua data yang telah sukses terbaca pada method read.
     * @param buffer variable untuk menampung data serial yang sukses terbaca.
     * @return ukuran atau size data serial.
     */
    size_t getBuffer(std::vector <unsigned char> &buffer);

    /**
     * @brief berfungsi untuk melakukan pengambilan jumlah data yang terdapat pada remaining buffer.
     *
     * Berfungsi untuk mengambil informasi nilai jumlah data yang terdapat pada remaining buffer.
     * @return ukuran atau size data serial dalam bytes.
     */
    size_t getRemainingDataSize();

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
     * @brief function overloading untuk __readStopBytes__ dengan parameter output menggunakan vector.
     *
     * Berfungsi untuk mengambil semua sisa data serial yang telah sukses terbaca diluar data buffer.
     * @param buffer variable untuk menampung data serial yang sukses terbaca.
     * @return ukuran atau size data serial.
     */
    size_t getRemainingBuffer(std::vector <unsigned char> &buffer);

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
     * @brief berfungsi untuk melakukan operasi penulisan data serial.
     *
     * Berfungsi untuk melakukan operasi penulisan data serial.
     * @param buffer data yang ingin ditulis.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika gagal melakukan penulisan data.
     */
    int writeData(const char *buffer);

    /**
     * @brief berfungsi untuk melakukan operasi penulisan data serial.
     *
     * Berfungsi untuk melakukan operasi penulisan data serial.
     * @param buffer data yang ingin ditulis.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika gagal melakukan penulisan data.
     */
    int writeData(const std::vector <unsigned char> buffer);

    /**
     * @brief berfungsi untuk melakukan operasi penulisan data serial.
     *
     * Berfungsi untuk melakukan operasi penulisan data serial.
     * @param buffer data yang ingin ditulis.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika gagal melakukan penulisan data.
     */
    int writeData(const std::string buffer);

    /**
     * @brief menutup port serial komunikasi.
     *
     * Berfungsi untuk melakukan penutupan port serial komunikasi
     */
    void closePort();
};

#endif
