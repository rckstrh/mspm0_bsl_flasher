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
    constexpr uint16_t data_len = 1;
    constexpr uint8_t ack_len = 1;

    constexpr uint8_t tx_buffer_len = header_len+crc_len+data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    // wrap packet
    fill_header(tx_buf, data_len);
    // cmd + data
    tx_buf[3] = static_cast<uint8_t> (BSL::CoreCmd::Connection);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;
    
    // test destroy crc check to check if ACK acts accordingly
    //buffer[5] = 0;

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < tx_buffer_len; i++) {
        printf("%x ", tx_buf[i]);
    }
    printf("\n");
#endif

    // write packet via uart
    write_buffer(serial, tx_buf, tx_buffer_len);

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
    constexpr uint16_t data_len = 1;

    constexpr uint8_t tx_buffer_len = header_len+crc_len+data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    // wrap packet
    fill_header(tx_buf, data_len);
    // cmd + data
    tx_buf[3] = static_cast<uint8_t> (BSL::CoreCmd::GetDeviceInfo);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;       

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < tx_buffer_len; i++) {
        printf("%x ", tx_buf[i]);
    }
    printf("\n");
#endif

    // write packet via uart
    write_buffer(serial, tx_buf, tx_buffer_len);

    // receive ACK,
    auto ack = receive_ack(serial);

    // receive device info
    BSL::_device_info device_info;
    constexpr uint16_t resp_data_len = 0x19;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read != rx_buffer_len)
        return {BSL::AckType::ERR_TIMEOUT, device_info};

#ifdef DEBUG_PRINT
    printf("Read %d bytes: ", bytes_read);
    for(int i=0; i < rx_buffer_len; i++) {
        printf("%x ", rx_buf[i]);
    }
    printf("\n");
#endif
    uint8_t* resp_code = rx_buf+header_len;
    uint8_t* resp_data = resp_code+1;

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

BSL::AckType BSL_UART::start_application()
{
    if(serial == nullptr)
        throw;

    // cmd specific
    constexpr uint16_t data_len = 1;

    constexpr uint8_t tx_buffer_len = header_len+crc_len+data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    // wrap packet
    fill_header(tx_buf, data_len);
    // cmd + data
    tx_buf[3] = static_cast<uint8_t> (BSL::CoreCmd::StartApplication);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+4)) = crc;       

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < tx_buffer_len; i++) {
        printf("%x ", tx_buf[i]);
    }
    printf("\n");
#endif

    // write packet via uart
    write_buffer(serial, tx_buf, tx_buffer_len);

    // receive ACK
    auto ack = receive_ack(serial);

    return ack;
}

std::tuple<BSL::AckType, BSL::RspMsg> BSL_UART::unlock_bootloader(const uint8_t* passwd)
{   
    if(serial == nullptr)
        throw;   

    // cmd specific
    constexpr uint16_t data_len = 1+password_len;

    constexpr uint8_t tx_buffer_len = header_len+crc_len+data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    fill_header(tx_buf, data_len);
    tx_buf[header_len] = static_cast<uint8_t> (BSL::CoreCmd::UnlockBootloader);
    memcpy(&tx_buf[header_len+1], passwd, password_len);
    
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;    

#ifdef DEBUG_PRINT
    printf("Writing: ");
    for(int i=0; i < tx_buffer_len; i++) {
        printf("%x ", tx_buf[i]);
    }
    printf("\n");
#endif 

    write_buffer(serial, tx_buf, tx_buffer_len);
    auto ack = receive_ack(serial);

    // receive core message
    BSL::RspMsg msg = BSL::RspMsg::BSL_UART_UNDEFINED;
    constexpr uint16_t resp_data_len = 0x02;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read != rx_buffer_len)
        return {BSL::AckType::ERR_TIMEOUT, msg};

#ifdef DEBUG_PRINT
    printf("Read %d bytes: ", bytes_read);
    for(int i=0; i < rx_buffer_len; i++) {
        printf("%x ", rx_buf[i]);
    }
    printf("\n");
#endif

    uint8_t* resp_code = rx_buf+header_len;
    if(static_cast<BSL::CoreRspCmd>(*resp_code) != BSL::CoreRspCmd::Message)
        return {ack, BSL::RspMsg::BSL_UART_UNDEFINED};

    uint8_t* resp_data = resp_code+1;

    msg = static_cast<BSL::RspMsg>(*resp_data);

    return {ack, msg};
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