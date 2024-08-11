#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include "serialink.hpp"
#include "virtuser.hpp"

extern void callbackEcho(VirtualSerial &ser, void *param);
extern void __callbackEchoWithDelay(VirtualSerial &ser, void *param);
extern void *callbackEchoWithDelay(void *ptr);

class SerialinkFramedDataTest:public::testing::Test {
protected:
    Serialink slave;
    VirtualSerial master;
    SerialinkFramedDataTest() : master(B115200, 10, 50) {}
    void SetUp() override {
        master.setCallback((const void *) &callbackEcho, nullptr);
    }

    void TearDown() override {
        // tidak ada kebutuhan post-set
    }
};

/* Default constructor */

TEST_F(SerialinkFramedDataTest, DefaultConstructor_1) {
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
    ASSERT_EQ(slave.getFormat(), nullptr);
    ASSERT_EQ(slave.readFramedData(), 3);
}
