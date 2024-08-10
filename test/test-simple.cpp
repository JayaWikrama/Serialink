#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "serial.hpp"
#include "virtuser.hpp"

void callbackEcho(VirtualSerial &ser, void *param){
    unsigned char buffer[1024];
    size_t sz = 0;
    int ret = 0;
    int *counter = (int *) param;
    ret = ser.readData();
    if (ret == 0){
        sz = ser.getBuffer(buffer, sizeof(buffer) - 1);
        if (sz > 0){
            buffer[sz] = 0x00;
            ser.writeData(buffer, sz);
        }
    }
}

void __callbackEchoWithDelay(VirtualSerial &ser, void *param){
    unsigned char buffer[1024];
    size_t sz = 0;
    int ret = 0;
    int *counter = (int *) param;
    ret = ser.readData();
    int mdelay = (int) (long) param;
    if (ret == 0){
        sz = ser.getBuffer(buffer, sizeof(buffer) - 1);
        if (sz > 0){
            buffer[sz] = 0x00;
            for (int i = 0; i < sz; i++){
                ser.writeData(buffer + i, 1);
                usleep(1000 * mdelay);
            }
        }
    }
}

void *callbackEchoWithDelay(void *ptr){
    VirtualSerial *ser = (VirtualSerial *) ptr;
    ser->setCallback((const void *) __callbackEchoWithDelay, (void *) 30);
    ser->begin();
    return NULL;
}

class SerialinkSimpleTest:public::testing::Test {
protected:
    Serial slave;
    VirtualSerial master;
    SerialinkSimpleTest() : master(B115200, 10, 50) {}
    void SetUp() override {
        master.setCallback((const void *) &callbackEcho, nullptr);
    }

    void TearDown() override {
        // tidak ada kebutuhan post-set
    }
};

/* Default constructor */

TEST_F(SerialinkSimpleTest, DefaultConstructor_1) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    ASSERT_EQ(slave.getPort(), std::string("/dev/ttyUSB0"));
    ASSERT_EQ(slave.getBaudrate(), B9600);
    ASSERT_EQ(slave.getTimeout(), 10);
    ASSERT_EQ(slave.getKeepAlive(), 0);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, CustomConstructor_1) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    Serial custom("/dev/myPORT0", B115200, 25);
    ASSERT_EQ(custom.getPort(), std::string("/dev/myPORT0"));
    ASSERT_EQ(custom.getBaudrate(), B115200);
    ASSERT_EQ(custom.getTimeout(), 25);
    ASSERT_EQ(custom.getKeepAlive(), 0);
    ASSERT_EQ(custom.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(custom.getBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
    ASSERT_EQ(custom.getRemainingDataSize(), 0);
    ASSERT_EQ(custom.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(custom.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, CustomConstructor_2) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    ASSERT_EQ(master.getPort(), std::string(""));
    ASSERT_NE(master.getVirtualPortName(), std::string(""));
    ASSERT_EQ(master.getBaudrate(), B115200);
    ASSERT_EQ(master.getTimeout(), 10);
    ASSERT_EQ(master.getKeepAlive(), 50);
    ASSERT_EQ(master.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(master.getBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
    ASSERT_EQ(master.getRemainingDataSize(), 0);
    ASSERT_EQ(master.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(master.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, CustomConstructor_3) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    Serial custom("/dev/myPORT0", B115200, 25, 12345);
    ASSERT_EQ(custom.getPort(), std::string("/dev/myPORT0"));
    ASSERT_EQ(custom.getBaudrate(), B115200);
    ASSERT_EQ(custom.getTimeout(), 25);
    ASSERT_EQ(custom.getKeepAlive(), 12345);
    ASSERT_EQ(custom.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(custom.getBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
    ASSERT_EQ(custom.getRemainingDataSize(), 0);
    ASSERT_EQ(custom.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(custom.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

/* Setter and Getter */
TEST_F(SerialinkSimpleTest, SetterGetter_1) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    slave.setPort("/dev/myPORT0");
    slave.setBaudrate(B19200);
    slave.setTimeout(25);
    slave.setKeepAlive(250);
    ASSERT_EQ(slave.getPort(), std::string("/dev/myPORT0"));
    ASSERT_EQ(slave.getBaudrate(), B19200);
    ASSERT_EQ(slave.getTimeout(), 25);
    ASSERT_EQ(slave.getKeepAlive(), 250);
    ASSERT_EQ(slave.getDataSize(), 0);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

/* write and read */
TEST_F(SerialinkSimpleTest, normalWriteAndRead_unknown_n_bytes) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 1000 && diffTime <= 1075, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_known_n_bytes) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readData(4), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_known_n_bytes_with_remaining_data) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\nabc", 7), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readData(4), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 3);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 3);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "abc", 3), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 3);
    ASSERT_EQ(tmp.size(), 3);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "abc", 3), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_with_delayed_bytes) {
    unsigned char buffer[8];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 0);
    pthread_create(&thread, NULL, callbackEchoWithDelay, (void *) &master);
    ASSERT_EQ(slave.readData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 120 && diffTime <= 220, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_startBytes) {
    unsigned char buffer[8];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "qwerty1234567890", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStartBytes((const unsigned char *) "1234", 4), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 6);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 6);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890", 6), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 6);
    ASSERT_EQ(tmp.size(), 6);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890", 6), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_startBytes_ov1) {
    unsigned char buffer[8];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "qwerty1234567890", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStartBytes("1234"), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 6);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 6);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890", 6), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 6);
    ASSERT_EQ(tmp.size(), 6);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890", 6), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_startBytes_ov2) {
    const char *data = "1234";
    unsigned char buffer[8];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "qwerty1234567890", 16), 0);
    ASSERT_EQ(master.begin(), true);
    tmp.assign((const unsigned char *) data, (const unsigned char *) data + strlen(data));
    ASSERT_EQ(slave.readStartBytes(tmp), 0);
    tmp.clear();
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 6);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 6);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890", 6), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 6);
    ASSERT_EQ(tmp.size(), 6);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890", 6), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_startBytes_ov3) {
    unsigned char buffer[8];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "qwerty1234567890", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStartBytes(std::string("1234")), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 6);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 6);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890", 6), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 6);
    ASSERT_EQ(tmp.size(), 6);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890", 6), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_untilStopBytes) {
    unsigned char buffer[512];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData(
      (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n",
      462
     ),
     0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readUntilStopBytes((const unsigned char *) "1234", 4), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 385);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 385);
    ASSERT_EQ(memcmp(buffer,
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getBuffer(tmp), 385);
    ASSERT_EQ(tmp.size(), 385);
    ASSERT_EQ(memcmp(tmp.data(),
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getRemainingDataSize(), 77);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 77);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 77);
    ASSERT_EQ(tmp.size(), 77);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_untilStopBytes_ov1) {
    unsigned char buffer[512];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData(
      (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n",
      462
     ),
     0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readUntilStopBytes("1234"), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 385);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 385);
    ASSERT_EQ(memcmp(buffer,
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getBuffer(tmp), 385);
    ASSERT_EQ(tmp.size(), 385);
    ASSERT_EQ(memcmp(tmp.data(),
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getRemainingDataSize(), 77);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 77);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 77);
    ASSERT_EQ(tmp.size(), 77);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_untilStopBytes_ov2) {
    const char *data = "1234";
    unsigned char buffer[512];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData(
      (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n",
      462
     ),
     0);
    ASSERT_EQ(master.begin(), true);
    tmp.assign((const unsigned char *) data, (const unsigned char *) data + strlen(data));
    ASSERT_EQ(slave.readUntilStopBytes(tmp), 0);
    tmp.clear();
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 385);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 385);
    ASSERT_EQ(memcmp(buffer,
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getBuffer(tmp), 385);
    ASSERT_EQ(tmp.size(), 385);
    ASSERT_EQ(memcmp(tmp.data(),
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getRemainingDataSize(), 77);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 77);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 77);
    ASSERT_EQ(tmp.size(), 77);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_untilStopBytes_ov3) {
    unsigned char buffer[512];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData(
      (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n",
      462
     ),
     0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readUntilStopBytes(std::string("1234")), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 385);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 385);
    ASSERT_EQ(memcmp(buffer,
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getBuffer(tmp), 385);
    ASSERT_EQ(tmp.size(), 385);
    ASSERT_EQ(memcmp(tmp.data(),
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getRemainingDataSize(), 77);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 77);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 77);
    ASSERT_EQ(tmp.size(), 77);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_stopBytes) {
    unsigned char buffer[32];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "1234567890qwerty", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStopBytes((const unsigned char *) "1234", 4), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 12);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890qwerty", 12), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890qwerty", 12), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_stopBytes_ov1) {
    unsigned char buffer[32];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "1234567890qwerty", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStopBytes("1234"), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 12);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890qwerty", 12), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890qwerty", 12), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_stopBytes_ov2) {
    const char *data = "1234";
    unsigned char buffer[32];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "1234567890qwerty", 16), 0);
    ASSERT_EQ(master.begin(), true);
    tmp.assign((const unsigned char *) data, (const unsigned char *) data + strlen(data));
    ASSERT_EQ(slave.readStopBytes(tmp), 0);
    tmp.clear();
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 12);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890qwerty", 12), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890qwerty", 12), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_stopBytes_ov3) {
    unsigned char buffer[32];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "1234567890qwerty", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStopBytes(std::string("1234")), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 12);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890qwerty", 12), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890qwerty", 12), 0);
}

TEST_F(SerialinkSimpleTest, normalWriteAndRead_nBytes) {
    unsigned char buffer[512];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData(
      (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234"
                              "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n",
      462
     ),
     0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readNBytes(385), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 385);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 385);
    ASSERT_EQ(memcmp(buffer,
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getBuffer(tmp), 385);
    ASSERT_EQ(tmp.size(), 385);
    ASSERT_EQ(memcmp(tmp.data(),
                     (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz    \n"
                                             "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz-1234",
                     385),
                    0);
    ASSERT_EQ(slave.getRemainingDataSize(), 77);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 77);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 77);
    ASSERT_EQ(tmp.size(), 77);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "qwertyuiopasdfghjklzxcvbnm09876543210987654321poiuytrewqlkjhgfdsamnbvcxz1234\n", 77), 0);
}

TEST_F(SerialinkSimpleTest, negativeWriteAndRead_port_not_open) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 1);
    ASSERT_EQ(slave.readData(), 1);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 10, true);
    ASSERT_EQ(slave.getDataSize(), 0);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
}

TEST_F(SerialinkSimpleTest, negativeWriteAndRead_invalid_port) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort("/dev/noPORT");
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 1);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 1);
    ASSERT_EQ(slave.readData(), 1);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 10, true);
    ASSERT_EQ(slave.getDataSize(), 0);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, negativeWriteAndRead_no_input_bytes_available) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\r\n\r\n", 4), 0);
    ASSERT_EQ(slave.readData(), 2);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 2500 && diffTime <= 2750, true);
    ASSERT_EQ(slave.getDataSize(), 0);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkSimpleTest, negativeWriteAndRead_stopBytes) {
    unsigned char buffer[32];
    pthread_t thread;
    std::vector <unsigned char> tmp;
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(50);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "qwerty1234567890", 16), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readStopBytes((const unsigned char *) "1234", 4), 3);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 16);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 16);
    ASSERT_NE(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 16);
    ASSERT_EQ(tmp.size(), 16);
    ASSERT_NE(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
