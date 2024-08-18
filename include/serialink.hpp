#ifndef __SERIALLINK_HPP__
#define __SERIALLINK_HPP__

#include "serial.hpp"
#include "data-frame.hpp"

class Serialink : public Serial {
  private:
    DataFrame *frameFormat;
  public:
    /**
     * @brief Default constructor.
     *
     * Berfungsi untuk melakukan setup private data dan parameter ke nilai default.
     */
    Serialink();

    /**
     * @brief Destructor.
     *
     * Berfungsi untuk melakukan release setiap memory yang dialokasikan.
     */
    ~Serialink();

    /**
     * @brief berfungsi untuk mengambil alamat memory frameFormat.
     *
     * Berfungsi untuk mengambil informasi alamat memory frameFormat.
     * @return alamat memory frameFormat.
     */
    DataFrame *getFormat();

    /**
     * @brief berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus.
     *
     * Berfungsi untuk melakukan operasi pembacaan data serial dengan format frame khusus. Data serial yang terbaca dapat diambil dengan method __Serial::getBuffer__.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     * @return 3 jika frame format belum di setup.
     * @return 4 jika terdapat format frame data yang tidak valid.
     */
    int readFramedData();

    /**
     * @brief berfungsi untuk melakukan operasi penulisan data serial dengan format frame khusus.
     *
     * Berfungsi untuk melakukan operasi penulisan data serial dengan format frame khusus.
     * @return 0 jika sukses.
     * @return 1 jika port belum terbuka.
     * @return 2 jika timeout.
     * @return 3 jika tidak ada data yang akan ditulis.
     */
    int writeFramedData();

    Serialink& operator=(const DataFrame &obj);

    Serialink& operator+=(const DataFrame &obj);

    Serialink& operator+(const DataFrame &obj);
};

#endif
