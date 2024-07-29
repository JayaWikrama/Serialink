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
     */
    int readFramedData();

    Serialink& operator=(const DataFrame &obj);

    Serialink& operator+=(const DataFrame &obj);

    Serialink& operator+(const DataFrame &obj);
};

#endif