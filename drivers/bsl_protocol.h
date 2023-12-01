/*
 * bsl_protocol.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include "stdint.h"
#include <unordered_map>

namespace BSL {

    /* packet format */
    struct _packet_format {
        uint8_t header;         // peripheral interface code
        uint8_t length[2];      // peripheral interface code
        uint8_t* bsl_core_data; // BSL core data
        uint8_t crc[4];         // peripheral interface code
    };

    static constexpr uint8_t CMD_HEADER = 0x80;
    static constexpr uint8_t RSP_HEADER = 0x08; 

    /* types of BSL acknowledgements */
    enum class AckType {
        BSL_ACK                         = 0x00,
        BSL_ERROR_HEADER_INCORRECT      = 0x51,
        BSL_ERROR_CHECKSUM_INCORRECT    = 0x52,
        BSL_ERROR_PACKET_SIZE_ZERO      = 0x53,
        BSL_ERROR_PACKET_SIZE_TOO_BIG   = 0x54,
        BSL_ERROR_UNKNOWN_ERROR         = 0x55,
        BSL_ERROR_UNKNOWN_BAUD_RATE     = 0x56,
        ERR_TIMEOUT                     = 0xA0,
        ERR_UNDEFINED                   = 0xA1
    };

    enum class CoreCmd {
        Connection              = 0x12, 
        UnlockBootloader        = 0x21,
        FlashRangeErase         = 0x23,
        MassErase               = 0x15,
        ProgramData             = 0x20,
        ProgramDataFast         = 0x24,
        MemoryRead              = 0x29,
        FactoryReset            = 0x30,
        GetDeviceInfo           = 0x19,
        StandaloneVerification  = 0x26,
        StartApplication        = 0x40,
        ChangeBaudrate          = 0x52
    };

    enum class CoreRspCmd {
        MemoryRead              = 0x30,
        GetDeviceInfo           = 0x31,
        StandaloneVerification  = 0x32,
        Message                 = 0x3B,
        DetailedError           = 0x3A
    };

    enum class RspMsg {
        SUCCESS                 = 0x00,     
        BSL_LOCKED              = 0x01, // BSL is not yet unlocked with Bootloader unlock password command or After BSL unlock
        BSL_PWD_ERR             = 0x02, // incorrect password has been sent to unlock bootloader
        BSL_MULTIPLE_PWD_ERR    = 0x03, // incorrect password has been sent to unlock bootloader 3 times
        UNKNOWN_CMD             = 0x04, // command given to the BSL was not recognized as valid command
        INVALID_MEM_RANGE       = 0x05, // given memory range is invalid
        INVALID_CMD             = 0x06, // command given to BSL is known command but it is invalid at that time instant and cannot be processed
        FACTORY_RESET_DISABLED  = 0x07, // factory reset is disabled in the BCR configuration
        FACTORY_RESET_PWD_ERR   = 0x08, // incorrect or no password has been sent with factory reset command
        READOUT_ERR             = 0x09, // memory read out be disabled in BCR configuration
        INV_ADDR_OR_LEN         = 0x0A, // start address or data length for the flash programming is not 8-byte aligned
        INV_LEN_VERIFICATION    = 0x0B  // data size sent for standalone verification is less than 1KB
    };

    struct _device_info {
        uint16_t cmd_interpreter_version;
        uint16_t build_id;
        uint32_t app_version;
        uint16_t plugin_if_version;
        uint16_t bsl_max_buff_size;
        uint32_t bsl_buff_start_addr;
        uint32_t bcr_conf_id;
        uint32_t bsl_conf_id;
    };

    class ResponseType {
        public:
            ResponseType(uint8_t _header, uint16_t _len, uint8_t _rsp, uint8_t* _data, bool _var_len) : 
                header(_header), rsp(_rsp), data(_data), variableLen(_var_len) 
                {
                    length[0] = (uint8_t) _len;
                    length[1] = (uint8_t) (_len >> 8);
                };

            uint8_t header;
            uint8_t length[2];
            uint8_t rsp;
            uint8_t* data;
            uint8_t crc[4];
            bool variableLen;
    };

    struct _core_data_info {
        //bool protected;
        uint8_t start_addr_len;
        int8_t data_len;
        ResponseType coreResponse;
    };

    /*
    static const std::unordered_map<CoreCmd, struct _core_data_info> CoreCommandMap = {
        {CoreCmd::Connection,               
            {.start_addr_len = 0, .data_len = 0, .coreResponse = false}},
        {CoreCmd::UnlockBootloader,         
            {.start_addr_len = 0, .data_len = 32, .coreResponse = true}},
        {CoreCmd::FlashRangeErase,          
            {.start_addr_len = 4, .data_len = 4, .coreResponse = true}},
        {CoreCmd::MassErase,                
            {.start_addr_len = 0, .data_len = 0, .coreResponse = true}},
        {CoreCmd::ProgramData,              
            {.start_addr_len = 4, .data_len = -1, .coreResponse = true}},
        {CoreCmd::ProgramDataFast,          
            {.start_addr_len = 4, .data_len = -1, .coreResponse = false}},
        {CoreCmd::MemoryRead,               
            {.start_addr_len = 4, .data_len = 4, .coreResponse = true}},
        {CoreCmd::FactoryReset,             
            {.start_addr_len = 0, .data_len = 16, .coreResponse = true}},
        {CoreCmd::GetDeviceInfo,            
            {.start_addr_len = 0, .data_len = 0, .coreResponse = true}},
        {CoreCmd::StandaloneVerification,   
            {.start_addr_len = 4, .data_len = 4, .coreResponse = true}},
        {CoreCmd::StartApplication,         
            {.start_addr_len = 0, .data_len = 0, .coreResponse = false}},
        {CoreCmd::ChangeBaudrate,           
            {.start_addr_len = 0, .data_len = 1, .coreResponse = false}}
    };
    */

    /*
    * from MSPM0 BSL example
    */
    #define CRC32_POLY 0xEDB88320
    static uint32_t softwareCRC(const char *data, uint8_t length)
    {
        uint32_t ii, jj, byte, crc, mask;
        ;

        crc = 0xFFFFFFFF;

        for (ii = 0; ii < length; ii++) {
            byte = data[ii];
            crc  = crc ^ byte;

            for (jj = 0; jj < 8; jj++) {
                mask = -(crc & 1);
                crc  = (crc >> 1) ^ (CRC32_POLY & mask);
            }
        }

        return crc;
    }

    static const char* AckTypeToString(AckType ack) 
    {
        switch(ack) {
        case AckType::BSL_ACK:
            return "ACK";                     
        case AckType::BSL_ERROR_HEADER_INCORRECT:
            return "Incorrect header";  
        case AckType::BSL_ERROR_CHECKSUM_INCORRECT:
            return "Wrong checksum";   
        case AckType::BSL_ERROR_PACKET_SIZE_ZERO:
            return "Packet size zero"; 
        case AckType::BSL_ERROR_PACKET_SIZE_TOO_BIG:
            return "Packet size too big";   
        case AckType::BSL_ERROR_UNKNOWN_ERROR:
            return "BSL unknown error";
        case AckType::BSL_ERROR_UNKNOWN_BAUD_RATE:
            return "Unknown baudrate";
        case AckType::ERR_TIMEOUT:
            return "Serial timeout";                    
        case AckType::ERR_UNDEFINED:
            return "UNDEFINED";   
        default:
            return "default undefined";
        }
    }
    
    static const char* DeviceInfoToString(struct _device_info device_info)
    {
        // TODO -> C++20 format maybe?
        return "";
    }

};