/*
 * bsl_uart.cpp
 *
 *  Created on: Dec 1, 2023
 *      Author: Jonas Rockstroh
 */
#include "bsl_uart.h"
#include <chrono>
#include <thread>

void fill_cmd_header(uint8_t* buffer, uint16_t data_len, BSL::CoreCmd cmd);
void fill_cmd_data(uint8_t *buffer, uint8_t *data, size_t data_len);
void write_buffer(Serial* serial, const uint8_t* buffer, size_t buffer_len);
BSL::AckType receive_ack(Serial* serial);

BSL_UART::BSL_UART(const char* _serial_port, int _verbose_level) : verbose_level(_verbose_level)
{
    serial = new Serial(_serial_port, verbose_level);
    if(!serial->_open()) {
       throw std::runtime_error("Failed to open serial port");
    }
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
    fill_cmd_header(tx_buf, data_len, BSL::CoreCmd::Connection);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;

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
    fill_cmd_header(tx_buf, data_len, BSL::CoreCmd::GetDeviceInfo);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;       

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

    set_bsl_max_buff_size(device_info.bsl_max_buff_size);

    return {ack, device_info};
}

void BSL_UART::set_bsl_max_buff_size(uint32_t _bsl_max_buff_size)
{
    bsl_max_buff_size = _bsl_max_buff_size;
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
    fill_cmd_header(tx_buf, data_len, BSL::CoreCmd::StartApplication);
    // calc crc over cmd+data
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+4)) = crc;       

    // write packet via uart
    write_buffer(serial, tx_buf, tx_buffer_len);

    // receive ACK
    auto ack = receive_ack(serial);

    return ack;
}

std::tuple<BSL::AckType, BSL::CoreMessage> BSL_UART::unlock_bootloader(const uint8_t* passwd)
{   
    if(serial == nullptr)
        throw;   

    // cmd specific
    constexpr uint16_t data_len = 1+password_len;

    constexpr uint8_t tx_buffer_len = header_len+crc_len+data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    fill_cmd_header(tx_buf, data_len, BSL::CoreCmd::UnlockBootloader);
    memcpy(&tx_buf[header_len+1], passwd, password_len);
    
    auto crc = BSL::softwareCRC(tx_buf+header_len, data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+data_len)) = crc;    

    write_buffer(serial, tx_buf, tx_buffer_len);
    auto ack = receive_ack(serial);

    // receive core message
    BSL::CoreMessage msg = BSL::CoreMessage::BSL_UART_UNDEFINED;
    constexpr uint16_t resp_data_len = 0x02;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read != rx_buffer_len)
        return {BSL::AckType::ERR_TIMEOUT, msg};

    uint8_t* resp_code = rx_buf+header_len;
    if(static_cast<BSL::CoreResponse>(*resp_code) != BSL::CoreResponse::Message)
        return {ack, BSL::CoreMessage::BSL_UART_UNDEFINED};

    uint8_t* resp_data = resp_code+1;

    msg = static_cast<BSL::CoreMessage>(*resp_data);

    return {ack, msg};
}

std::tuple<BSL::AckType, BSL::CoreMessage> BSL_UART::readback_data(const uint32_t addr, const uint32_t readback_len, uint8_t *dst)
{
    if(serial == nullptr)
        throw;  

    auto ack = BSL::AckType::ERR_UNDEFINED;
    auto msg = BSL::CoreMessage::BSL_UART_UNDEFINED;

    constexpr uint16_t tx_data_len = 9;
    constexpr uint8_t tx_buffer_len = header_len+crc_len+tx_data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    fill_cmd_header(tx_buf, tx_data_len, BSL::CoreCmd::MemoryRead);

    *((uint32_t*) (&tx_buf[header_len+1])) = addr; 
    *((uint32_t*) (&tx_buf[header_len+5])) = readback_len; 
    
    auto crc = BSL::softwareCRC(tx_buf+header_len, tx_data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+tx_data_len)) = crc;

    write_buffer(serial, tx_buf, tx_buffer_len);
    ack = receive_ack(serial);

    if(ack != BSL::AckType::BSL_ACK) {
        return {ack, msg};
    }

    const uint16_t resp_data_len = 1+readback_len;
    const uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    // read header, length and rsp code first to check if read is valid
    bytes_read = serial->readBytes((char*) rx_buf, 9);
    if(bytes_read != 9)
        return {BSL::AckType::ERR_TIMEOUT, msg};

    uint8_t resp_code = *(rx_buf+header_len);

    if(static_cast<BSL::CoreResponse>(resp_code) != BSL::CoreResponse::MemoryRead) {
        printf("Failed to read. Reason: %s\n", BSL::CoreMessageToString(static_cast<BSL::CoreMessage>(*(rx_buf+header_len+1))));
    }

    // implement read out of memory if BCR configuration allows it
    // postponed for now.


    return {ack, msg};
}

BSL::AckType BSL_UART::change_baudrate(BSL::Baudrate rate)
{
    auto ack = BSL::AckType::ERR_UNDEFINED;

    constexpr uint16_t tx_data_len = 2;
    constexpr uint8_t tx_buffer_len = header_len+crc_len+tx_data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    uint8_t cmd_data[1] = {static_cast<uint8_t>(rate)};

    // wrap packet
    fill_cmd_header(tx_buf, tx_data_len, BSL::CoreCmd::ChangeBaudrate);
    fill_cmd_data(tx_buf, cmd_data, 1);
    auto crc = BSL::softwareCRC(tx_buf+header_len, tx_data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+tx_data_len)) = crc;

    // write and get ack
    write_buffer(serial, tx_buf, tx_buffer_len);
    ack = receive_ack(serial);

    if(ack == BSL::AckType::BSL_ACK) {
        serial->_close();
        serial->_open(BSL::BSLBaudToSerialBaud(rate));
    }

    return ack;
}

std::tuple<BSL::AckType, BSL::CoreMessage, uint32_t> BSL_UART::verify(const uint32_t addr, const uint32_t size)
{
    auto ack = BSL::AckType::ERR_UNDEFINED;
    auto msg = BSL::CoreMessage::BSL_UART_UNDEFINED;
    uint32_t mem_block_crc = 0;

    constexpr uint16_t tx_data_len = 9;
    constexpr uint8_t tx_buffer_len = header_len+crc_len+tx_data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    uint32_t cmd_data[2];
    cmd_data[0] = addr;
    cmd_data[1] = size;

    // wrap packet
    fill_cmd_header(tx_buf, tx_data_len, BSL::CoreCmd::StandaloneVerification);
    fill_cmd_data(tx_buf, (uint8_t*) cmd_data, 8);
    auto crc = BSL::softwareCRC(tx_buf+header_len, tx_data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+tx_data_len)) = crc;

    // write and get ack
    write_buffer(serial, tx_buf, tx_buffer_len);
    ack = receive_ack(serial);


    // receive and check if standalone msg or core message
    constexpr uint16_t resp_data_len = 0x05;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, header_len+2);
    if(bytes_read != header_len+2)
        return {BSL::AckType::ERR_TIMEOUT, msg, mem_block_crc};

    uint8_t* resp_code = rx_buf+header_len;
    if(static_cast<BSL::CoreResponse>(*resp_code) != BSL::CoreResponse::StandaloneVerification)
        return {ack, static_cast<BSL::CoreMessage>(*(resp_code+1)), 0};

    // read rest of standalone response
    bytes_read += serial->readBytes((char*) rx_buf+bytes_read, rx_buffer_len-bytes_read);
    if(bytes_read != rx_buffer_len)
        return {BSL::AckType::ERR_TIMEOUT, msg, mem_block_crc};

    uint8_t* resp_data = resp_code+1;

    mem_block_crc = *((uint32_t*) resp_data);


    return {ack, BSL::CoreMessage::SUCCESS, mem_block_crc};
}

std::tuple<BSL::AckType, BSL::CoreMessage> BSL_UART::program_data(const uint32_t addr, const uint8_t* program_data, size_t program_size)
{
    auto ack = BSL::AckType::ERR_UNDEFINED;
    auto msg = BSL::CoreMessage::BSL_UART_UNDEFINED;   

    // addr and program size have to be 8 byte aligned
    if((addr % 8) != 0) {
        printf("program addr needs to be 8byte aligned! Canceling.\n");
        return {ack, msg};
    }

    if((program_size % 8) != 0) {
        printf("program size needs to be 8byte aligned! Canceling.\n");
        return {ack, msg};
    }

    // for now only handle data that is smaller than bsl max buffer size
    // let segmentation happen outside this function, maybe move it in here after that works
    // => actually do not know if this is true when we only send 128byte payloads anyway, so try without
    /*
    if(program_size > bsl_max_buff_size) {
        printf("program_data size > bsl_max_buffer_size. Canceling.\n");
        return {ack, msg};
    }
    */

    uint32_t bytes_to_write = program_size;
    uint32_t bytes_written = 0;
    uint16_t data_block_size = 0;
    uint32_t block_count = 0;

    while(bytes_to_write > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        if (bytes_to_write >= MAX_PAYLOAD_SIZE)
            data_block_size = MAX_PAYLOAD_SIZE;
        else
            data_block_size = bytes_to_write;

        bytes_to_write -= data_block_size;
    

        const uint32_t tx_data_len = 1+4+data_block_size;
        const uint32_t tx_buffer_len = header_len+crc_len+tx_data_len;
        uint8_t tx_buf[tx_buffer_len] = {0};

        uint8_t* addr_field = tx_buf+header_len+cmd_len;
        uint8_t* tx_program_data_start = addr_field+addr_len;

        // wrap packet
        fill_cmd_header(tx_buf, tx_data_len, BSL::CoreCmd::ProgramData);
        //fill_cmd_data(tx_buf, cmd_data, 8);
        *((uint32_t*) addr_field) = addr+bytes_written;
        memcpy(tx_program_data_start, program_data+bytes_written, data_block_size);
        auto crc = BSL::softwareCRC(tx_buf+header_len, tx_data_len);
        // append crc
        *((uint32_t*) (tx_buf+header_len+tx_data_len)) = crc;

        // write and get ack
        write_buffer(serial, tx_buf, tx_buffer_len);
        ack = receive_ack(serial);   

        msg = receive_core_message();

        //return immediately if block was written unsuccessfully
        if((ack != BSL::AckType::BSL_ACK) || (msg != BSL::CoreMessage::SUCCESS)) {
            printf("Programming failed at block %d, addr 0x%08x\n", block_count, addr+bytes_written);
            return {ack, msg};
        }

        bytes_written += data_block_size;
        block_count++;
    }


    return {ack, msg};
}

std::tuple<BSL::AckType, BSL::CoreMessage> BSL_UART::mass_erase()
{
    auto ack = BSL::AckType::ERR_UNDEFINED;
    auto msg = BSL::CoreMessage::BSL_UART_UNDEFINED;      

    const uint16_t tx_data_len = 1;
    const uint8_t tx_buffer_len = header_len+crc_len+tx_data_len;
    uint8_t tx_buf[tx_buffer_len] = {0};

    // wrap packet
    fill_cmd_header(tx_buf, tx_data_len, BSL::CoreCmd::MassErase);
    auto crc = BSL::softwareCRC(tx_buf+header_len, tx_data_len);
    // append crc
    *((uint32_t*) (tx_buf+header_len+tx_data_len)) = crc;

    // write and get ack
    write_buffer(serial, tx_buf, tx_buffer_len);
    ack = receive_ack(serial);    

    // receive core message
    constexpr uint16_t resp_data_len = 0x02;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read != rx_buffer_len)
        return {BSL::AckType::ERR_TIMEOUT, msg};

    uint8_t* resp_code = rx_buf+header_len;
    if(static_cast<BSL::CoreResponse>(*resp_code) != BSL::CoreResponse::Message)
        return {ack, BSL::CoreMessage::BSL_UART_UNDEFINED};

    uint8_t* resp_data = resp_code+1;
    
    msg = static_cast<BSL::CoreMessage>(*resp_data);

    return {ack, msg};
}

void fill_cmd_header(uint8_t* buffer, uint16_t data_len, BSL::CoreCmd cmd)
{
    // header + data length
    *buffer = BSL::CMD_HEADER; 
    *((uint16_t *)(buffer+1)) = data_len;
    *(buffer+3) = static_cast<uint8_t>(cmd);
}

void fill_cmd_data(uint8_t *buffer, uint8_t *data, size_t data_len)
{
    memcpy(buffer+4, data, data_len);
}

void write_buffer(Serial* serial, const uint8_t* buffer, size_t buffer_len)
{
    int bytesWritten = 0;
    bytesWritten = serial->writeBytes((const char*) buffer, buffer_len);
    if(bytesWritten != buffer_len) {
        printf("Error writing, not enough bytes written\n");
    }
}

BSL::AckType receive_ack(Serial* serial)
{
    // receive ACK
    BSL::AckType ack;
    char rxByte = 0xFF;
    int bytes_read = 0;
    bytes_read = serial->readBytes(&rxByte, 1);
    if(bytes_read == 0) {
        return BSL::AckType::ERR_TIMEOUT;
    }

    if(bytes_read < 0)
        return BSL::AckType::ERR_UNDEFINED;

    ack = static_cast<BSL::AckType>(rxByte);

    return ack;
}

BSL::CoreMessage BSL_UART::receive_core_message()
{
    // receive core message
    constexpr uint16_t resp_data_len = 0x02;
    constexpr uint16_t rx_buffer_len = header_len+resp_data_len+crc_len;
    uint8_t rx_buf[rx_buffer_len] = {0};
    int bytes_read = 0;

    bytes_read = serial->readBytes((char*) rx_buf, rx_buffer_len);
    if(bytes_read != rx_buffer_len)
        return BSL::CoreMessage::BSL_UART_UNDEFINED;

    uint8_t* resp_code = rx_buf+header_len;
    if(static_cast<BSL::CoreResponse>(*resp_code) != BSL::CoreResponse::Message)
        return BSL::CoreMessage::BSL_UART_UNDEFINED;

    uint8_t* resp_data = resp_code+1;
    
    return static_cast<BSL::CoreMessage>(*resp_data);

}