/*
 * bsl_uart.cpp
 *
 *  Created on: Dec 1, 2023
 *      Author: Jonas Rockstroh
 */
#include "bsl_uart.h"

#define DEBUG_PRINT

void fill_header(uint8_t* buffer, uint16_t data_len);
void write_buffer(Serial* serial, const uint8_t* buffer, size_t buffer_len);
BSL::AckType receive_ack(Serial* serial);

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

    // cmd specific
    constexpr uint16_t data_length = 1;
    constexpr uint8_t ack_len = 1;

    constexpr uint8_t bufferLen = header_len+crc_len+data_length;
    uint8_t buffer[bufferLen] = {0};

    // wrap packet
    fill_header(buffer, data_length);
    // cmd + data
    buffer[3] = static_cast<uint8_t> (BSL::CoreCmd::Connection);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC((const char*)buffer+header_len, data_length);
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
    write_buffer(serial, buffer, bufferLen);

    // receive ACK,
    // connection cmd does not send additional response data
    auto ack = receive_ack(serial);

    return ack;
}

std::tuple<BSL::AckType, BSL::_device_info> BSL_UART::get_device_info()
{
    if(serial == nullptr)
        throw;

    // cmd specific
    constexpr uint16_t data_length = 1;

    constexpr uint8_t bufferLen = header_len+crc_len+data_length;
    uint8_t buffer[bufferLen] = {0};

    // wrap packet
    fill_header(buffer, data_length);
    // cmd + data
    buffer[3] = static_cast<uint8_t> (BSL::CoreCmd::GetDeviceInfo);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(((const char*) buffer)+header_len, data_length);
    // append crc
    *((uint32_t*) (buffer+4)) = crc;       

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < bufferLen; i++) {
        printf("%x ", buffer[i]);
    }
    printf("\n");
#endif

    // write packet via uart
    write_buffer(serial, buffer, bufferLen);

    // receive ACK,
    auto ack = receive_ack(serial);

    // receive device info
    BSL::_device_info device_info;
    constexpr uint16_t resp_data_len = 0x19;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;
    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read == 0) {
        printf("Response read timeout\n");
    }

    if(bytes_read < 0)
        throw;

#ifdef DEBUG_PRINT
    printf("Read: ");
    for(int i=0; i < rx_buffer_len; i++) {
        printf("%x ", rx_buf[i]);
    }
    printf("\n");
#endif
    uint8_t* resp_data = rx_buf+header_len;

    device_info.cmd_interpreter_version = *((uint16_t *)(&resp_data[0]));
    device_info.build_id = *((uint16_t *)(&resp_data[2]));
    device_info.app_version = *((uint32_t *)(&resp_data[4]));
    device_info.plugin_if_version = *((uint16_t *)(&resp_data[8]));
    device_info.bsl_max_buff_size = *((uint16_t *)(&resp_data[10]));
    device_info.bsl_buff_start_addr = *((uint32_t *)(&resp_data[12]));
    device_info.bcr_conf_id = *((uint32_t *)(&resp_data[16]));
    device_info.bsl_conf_id = *((uint32_t *)(&resp_data[20]));

    return {ack, device_info};
}

inline void fill_header(uint8_t* buffer, uint16_t data_len)
{
    // header + data length
    *buffer = BSL::CMD_HEADER; 
    *((uint16_t *)(buffer+1)) = data_len;
}

inline void write_buffer(Serial* serial, const uint8_t* buffer, size_t buffer_len)
{
    int bytesWritten = 0;
    bytesWritten = serial->writeBytes((const char*) buffer, buffer_len);
    if(bytesWritten != buffer_len) {
        printf("Error writing, not enough bytes written\n");
    }
}

BSL::AckType receive_ack(Serial* serial)
{
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
    if(bytes_read == 1) {
        printf("Read: %x\n", (uint8_t) ack);
    }
#endif

    return ack;
}