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

/* Operator Overloading */

TEST_F(SerialinkFramedDataTest, OperatorOverloading_1) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &callbackEcho, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &callbackEcho) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
}

TEST_F(SerialinkFramedDataTest, OperatorOverloading_2) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &callbackEcho, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes;
    slave += cmdBytes;
    slave += dataBytes;
    slave += stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &callbackEcho) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
}