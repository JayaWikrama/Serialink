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

void setupLengthByCommand(DataFrame &frame, void *ptr){
    int data = 0;
    DataFrame *target = frame.getNext();
    Serialink *obj = (Serialink *) ptr;
    if (target == nullptr) return;
    frame.getData((unsigned char *) &data, 1);
    if (data == 0x35)
        target->setSize(3);
    else if (data == 0x36)
        target->setSize(2);
    else
        obj->trigInvDataIndicator();
}

void setupLengthByCommand2(DataFrame &frame, void *ptr){
    int data = 0;
    Serialink *obj = (Serialink *) ptr;
    DataFrame *target = frame.getNext()->getNext()->getNext()->getNext();
    if (target == nullptr) return;
    frame.getData((unsigned char *) &data, 1);
    if (data > 3){
        target->setSize(data - 3);
    }
    else {
        obj->trigInvDataIndicator();
    }
}

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

TEST_F(SerialinkFramedDataTest, OperatorOverloading_3) {
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
    DataFrame *frame = nullptr;
    frame = slave[0];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_START_BYTES);
    frame = slave[1];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_COMMAND);
    frame = slave[2];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_DATA);
    frame = slave[3];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_STOP_BYTES);
    frame = slave[4];
    ASSERT_EQ(frame, nullptr);
}

TEST_F(SerialinkFramedDataTest, OperatorOverloading_4) {
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
    DataFrame *frame = nullptr;
    frame = slave[DataFrame::FRAME_TYPE_START_BYTES];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getSize(), 4);
    frame = slave[DataFrame::FRAME_TYPE_COMMAND];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getSize(), 1);
    frame = slave[DataFrame::FRAME_TYPE_DATA];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getSize(), 0);
    frame = slave[DataFrame::FRAME_TYPE_STOP_BYTES];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getSize(), 4);
    frame = slave[DataFrame::FRAME_TYPE_VALIDATOR];
    ASSERT_EQ(frame, nullptr);
}

TEST_F(SerialinkFramedDataTest, OperatorOverloading_5) {
    unsigned char buffer[8];
    std::vector <unsigned char> tmp;
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes0(DataFrame::FRAME_TYPE_COMMAND, 1);
    DataFrame dataBytes0(DataFrame::FRAME_TYPE_DATA, 1);
    DataFrame cmdBytes1(DataFrame::FRAME_TYPE_COMMAND, 2);
    DataFrame dataBytes1(DataFrame::FRAME_TYPE_DATA, 2);
    DataFrame cmdBytes2(DataFrame::FRAME_TYPE_COMMAND, 3);
    DataFrame dataBytes2(DataFrame::FRAME_TYPE_DATA, 3);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes0 + dataBytes0 + cmdBytes1 + dataBytes1 + cmdBytes2 + dataBytes2 + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:2]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:2]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:3]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:3]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    DataFrame *frame = nullptr;
    frame = slave[{DataFrame::FRAME_TYPE_START_BYTES, 0}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_START_BYTES);
    ASSERT_EQ(frame->getSize(), 4);
    frame = slave[{DataFrame::FRAME_TYPE_COMMAND, 0}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_COMMAND);
    ASSERT_EQ(frame->getSize(), 1);
    frame = slave[{DataFrame::FRAME_TYPE_DATA, 0}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_DATA);
    ASSERT_EQ(frame->getSize(), 1);
    frame = slave[{DataFrame::FRAME_TYPE_COMMAND, 1}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_COMMAND);
    ASSERT_EQ(frame->getSize(), 2);
    frame = slave[{DataFrame::FRAME_TYPE_DATA, 1}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_DATA);
    ASSERT_EQ(frame->getSize(), 2);
    frame = slave[{DataFrame::FRAME_TYPE_COMMAND, 2}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_COMMAND);
    ASSERT_EQ(frame->getSize(), 3);
    frame = slave[{DataFrame::FRAME_TYPE_DATA, 2}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_DATA);
    ASSERT_EQ(frame->getSize(), 3);
    frame = slave[{DataFrame::FRAME_TYPE_STOP_BYTES}];
    ASSERT_NE(frame, nullptr);
    ASSERT_EQ(frame->getType(), DataFrame::FRAME_TYPE_STOP_BYTES);
    ASSERT_EQ(frame->getSize(), 4);
    frame = slave[{DataFrame::FRAME_TYPE_VALIDATOR}];
    ASSERT_EQ(frame, nullptr);
}

/* Read Test */

TEST_F(SerialinkFramedDataTest, WriteTest_1) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, "5");
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA, "678");
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<35>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:3]:<<363738>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeFramedData(), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_1) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("1234567890-="), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withSuffix_1) {
    unsigned char buffer[64];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("1234567890-=qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 60);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 60);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 60);
    ASSERT_EQ(tmp.size(), 60);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withPrefixAndSuffix_1) {
    unsigned char buffer[64];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("qwertyuioplkjhgfdsazxcvbnmqwertyuioplkjhgfdsazxcvbnm1234567890-=qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 60);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 60);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 60);
    ASSERT_EQ(tmp.size(), 60);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withPrefixAndSuffix_2) {
    unsigned char buffer[64];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("m1234567890-=qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 60);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 60);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 60);
    ASSERT_EQ(tmp.size(), 60);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withPrefixAndSuffix_3) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    int i = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, nullptr);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("qwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    for (i = 0; i < 4; i++){
        gettimeofday(&tvStart, NULL);
        ASSERT_EQ(slave.readFramedData(), 0);
        gettimeofday(&tvEnd, NULL);
        diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
        ASSERT_EQ(diffTime == 0, true);
        ASSERT_EQ(slave.getDataSize(), 12);
        ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
        ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
        ASSERT_EQ(slave.getBuffer(tmp), 12);
        ASSERT_EQ(tmp.size(), 12);
        ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    }
    ASSERT_EQ(slave.getRemainingDataSize(), 7);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 7);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 7);
    ASSERT_EQ(tmp.size(), 7);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "zxcvbnm", 7), 0);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.readFramedData(), 2);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 2400 && diffTime <= 2800, true);
    ASSERT_EQ(slave.getDataSize(), 7);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 7);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 7);
    ASSERT_EQ(tmp.size(), 7);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withPrefixAndSuffix_4) {
    unsigned char buffer[64];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, 1, (const unsigned char *) "\x02");
    DataFrame lengthBytes(DataFrame::FRAME_TYPE_CONTENT_LENGTH, 1);
    lengthBytes.setPostExecuteFunction((const void *) &setupLengthByCommand2, (void *) &slave);
    DataFrame canBytes(DataFrame::FRAME_TYPE_DATA, 1);
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    DataFrame idBytes(DataFrame::FRAME_TYPE_DATA, 1);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame crcBytes(DataFrame::FRAME_TYPE_VALIDATOR, 2);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, 1, (const unsigned char *) "\x03");
    slave = startBytes + lengthBytes + canBytes + cmdBytes + idBytes + dataBytes + crcBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:1]:<<02>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_CONTENT_LENGTH[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand2) + ">>\n"
              "FRAME_TYPE_DATA[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_VALIDATOR[size:2]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:1]:<<03>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData((const unsigned char *) "\x02\x05\x13\x14\x02\x05\x13\x14\x02\x05\x13\x14\x02\x05\x13\x14\x02\x05\x13\x14\x02\x05\x13\x14\x02\x05\x13\x14\x03\x02\x05\x13\x14\x02\x05\x13\x14\x00\x12\x12\x13\x14\x03", 43), 0);
    ASSERT_EQ(master.begin(), true);
    for (int i = 0; i < 8; i++){
        ASSERT_EQ(slave.readFramedData(), 2);
        ASSERT_EQ(slave.getDataSize(), 1);
        ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 1);
        ASSERT_EQ(buffer[0], 0x02);
        ASSERT_EQ(slave.getBuffer(tmp), 1);
        ASSERT_EQ(tmp.size(), 1);
        ASSERT_EQ(tmp[0], 0x02);
    }
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 10);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 10);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "\x02\x05\x13\x14\x00\x12\x12\x13\x14\x03", 10), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 10);
    ASSERT_EQ(tmp.size(), 10);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "\x02\x05\x13\x14\x00\x12\x12\x13\x14\x03", 10), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withUnknownDataSz_1) {
    unsigned char buffer[64];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("m1234567890-=qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 0);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 12);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 12);
    ASSERT_EQ(tmp.size(), 12);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 60);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 60);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 60);
    ASSERT_EQ(tmp.size(), 60);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "qwertyuiopplkjhgfdsaZxcvbh76redcvbnm,mvdswertyuioiuhgfcxvbnm", 60), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTest_withUnknownDataSz_2) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    int i = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("qwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnmqwertyuiop[1234567890-=zxcvbnm"), 0);
    ASSERT_EQ(master.begin(), true);
    for (i = 0; i < 4; i++){
        gettimeofday(&tvStart, NULL);
        ASSERT_EQ(slave.readFramedData(), 0);
        gettimeofday(&tvEnd, NULL);
        diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
        ASSERT_EQ(diffTime == 0, true);
        ASSERT_EQ(slave.getDataSize(), 12);
        ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 12);
        ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234567890-=", 12), 0);
        ASSERT_EQ(slave.getBuffer(tmp), 12);
        ASSERT_EQ(tmp.size(), 12);
        ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234567890-=", 12), 0);
    }
    ASSERT_EQ(slave.getRemainingDataSize(), 7);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 7);
    ASSERT_EQ(memcmp(buffer, (unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 7);
    ASSERT_EQ(tmp.size(), 7);
    ASSERT_EQ(memcmp(tmp.data(), (unsigned char *) "zxcvbnm", 7), 0);
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.readFramedData(), 2);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 2400 && diffTime <= 2800, true);
    ASSERT_EQ(slave.getDataSize(), 7);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 7);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 7);
    ASSERT_EQ(tmp.size(), 7);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "zxcvbnm", 7), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 0);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 0);
    ASSERT_EQ(tmp.size(), 0);
}

TEST_F(SerialinkFramedDataTest, ReadNegativeTest_1) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("1234567890-="), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 4);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 4);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "1234", 4), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 8);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 8);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "567890-=", 8), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 8);
    ASSERT_EQ(tmp.size(), 8);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "567890-=", 8), 0);
}

TEST_F(SerialinkFramedDataTest, ReadNegativeTest_2) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA, 3);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES);
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_DATA[size:3]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("1234567890-="), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 4);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 8);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 8);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "12345678", 8), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 8);
    ASSERT_EQ(tmp.size(), 8);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "12345678", 8), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 4);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 4);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "90-=", 4), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 4);
    ASSERT_EQ(tmp.size(), 4);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "90-=", 4), 0);
}

TEST_F(SerialinkFramedDataTest, ReadTestNegative_InvalidTrigger_1) {
    unsigned char buffer[16];
    struct timeval tvStart, tvEnd;
    int diffTime = 0;
    std::vector <unsigned char> tmp;
    slave.setPort(master.getVirtualPortName());
    slave.setBaudrate(B115200);
    slave.setTimeout(25);
    slave.setKeepAlive(1000);
    DataFrame startBytes(DataFrame::FRAME_TYPE_START_BYTES, "1234");
    DataFrame cmdBytes(DataFrame::FRAME_TYPE_COMMAND, 1);
    cmdBytes.setPostExecuteFunction((const void *) &setupLengthByCommand, (void *) &slave);
    DataFrame dataBytes(DataFrame::FRAME_TYPE_DATA);
    DataFrame stopBytes(DataFrame::FRAME_TYPE_STOP_BYTES, "90-=");
    slave = startBytes + cmdBytes + dataBytes + stopBytes;
    ASSERT_EQ(slave.getFormat()->getDataFrameFormat(),
              "FRAME_TYPE_START_BYTES[size:4]:<<31323334>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_COMMAND[size:1]:<<>><<exeFunc:0>><<postFunc:" + std::to_string((unsigned long) &setupLengthByCommand) + ">>\n"
              "FRAME_TYPE_DATA[size:0]:<<>><<exeFunc:0>><<postFunc:0>>\n"
              "FRAME_TYPE_STOP_BYTES[size:4]:<<39302D3D>><<exeFunc:0>><<postFunc:0>>\n");
    gettimeofday(&tvStart, NULL);
    ASSERT_EQ(slave.openPort(), 0);
    ASSERT_EQ(slave.writeData("1234467890-="), 0);
    ASSERT_EQ(master.begin(), true);
    ASSERT_EQ(slave.readFramedData(), 4);
    gettimeofday(&tvEnd, NULL);
    diffTime = (tvEnd.tv_sec - tvStart.tv_sec) * 1000 + (tvEnd.tv_usec - tvStart.tv_usec) / 1000;
    ASSERT_EQ(diffTime >= 0 && diffTime <= 75, true);
    ASSERT_EQ(slave.getDataSize(), 5);
    ASSERT_EQ(slave.getBuffer(buffer, sizeof(buffer)), 5);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "12344", 5), 0);
    ASSERT_EQ(slave.getBuffer(tmp), 5);
    ASSERT_EQ(tmp.size(), 5);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "12344", 5), 0);
    ASSERT_EQ(slave.getRemainingDataSize(), 7);
    ASSERT_EQ(slave.getRemainingBuffer(buffer, sizeof(buffer)), 7);
    ASSERT_EQ(memcmp(buffer, (const unsigned char *) "67890-=", 7), 0);
    ASSERT_EQ(slave.getRemainingBuffer(tmp), 7);
    ASSERT_EQ(tmp.size(), 7);
    ASSERT_EQ(memcmp(tmp.data(), (const unsigned char *) "67890-=", 7), 0);
}
