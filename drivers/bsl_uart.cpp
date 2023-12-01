/*
 * bsl_uart.cpp
 *
 *  Created on: Dec 1, 2023
 *      Author: Jonas Rockstroh
 */
#include "bsl_uart.h"

//#define DEBUG_PRINT

void fillHeader(uint8_t* buffer, uint16_t data_len);

BSL_UART::BSL_UART(const char* _serial_port)
{
    serial = new Serial(_serial_port);
    serial->_open();
}

BSL_UART::~BSL_UART() 
{
    delete serial;
}

BSL::AckType BSL_UART::connect()
{
    if(serial == nullptr)
        throw;

    // "cmd global" const stuff
    constexpr uint8_t header_len = 3;
    constexpr uint8_t crc_len = 4;

    // cmd specific
    constexpr uint16_t data_length = 1;
    constexpr uint8_t ack_len = 1;

    constexpr uint8_t bufferLen = header_len+crc_len+data_length;
    uint8_t buffer[bufferLen] = {0};

    // wrap packet
    fillHeader(buffer, data_length);
    // cmd + data
    buffer[3] = 0x12;
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(buffer+header_len, data_length);
    // append crc
    *((uint32_t*) (buffer+4)) = crc;
    
    // test destroy crc check to check if ACK acts accordingly
    //buffer[5] = 0;

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < bufferLen; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
#endif

    // write packet via uart
    int bytesWritten = 0;
    bytesWritten = serial->writeBytes((char*) buffer, bufferLen);
    if(bytesWritten != bufferLen) {
        printf("Error writing, not enough bytes written\n");
    }

    // receive ACK,
    // connection cmd does not send additional response data
    BSL::AckType ack;
    char rxByte = 0xFF;
    int bytes_read = 0;
    bytes_read = serial->readBytes(&rxByte, 1);
    if(bytes_read == 0) {
        //printf("Read timeout\n");
        return BSL::AckType::ERR_TIMEOUT;
    }

    if(bytes_read < 0)
        return BSL::AckType::ERR_UNDEFINED;

    ack = static_cast<BSL::AckType>(rxByte);

#ifdef DEBUG_PRINT
    if(bytes_read == ack_len) {
        printf("Read: %x\n", (uint8_t) ack);
    }
#endif

    return ack;
}

inline void fillHeader(uint8_t* buffer, uint16_t data_len)
{
    // header + data length
    *buffer = 0x80; 
    *((uint16_t *)(buffer+1)) = data_len;
}