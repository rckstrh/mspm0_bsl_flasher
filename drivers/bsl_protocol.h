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

    enum class CoreResponse {
        MemoryRead              = 0x30,
        GetDeviceInfo           = 0x31,
        StandaloneVerification  = 0x32,
        Message                 = 0x3B,
        DetailedError           = 0x3A
    };

    enum class CoreMessage {
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
        INV_LEN_VERIFICATION    = 0x0B, // data size sent for standalone verification is less than 1KB
        BSL_UART_UNDEFINED      = 0xDD
    };

    
    enum class Baudrate {
        BSL_B4800       = 0x1,
        BSL_B9600       = 0x2,
        BSL_B19200      = 0x3,
        BSL_B38400      = 0x4,
        BSL_B57600      = 0x5,
        BSL_B115200     = 0x6,
        BSL_B1000000    = 0x7,
        BSL_B2000000    = 0x8,
        BSL_B3000000    = 0x9
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

    /*
    * from MSPM0 BSL example
    */
    #define CRC32_POLY 0xEDB88320
    inline uint32_t softwareCRC(const uint8_t *data, uint8_t length)
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

    static const char* CoreMessageToString(CoreMessage msg) 
    {
        switch(msg) {
        case CoreMessage::SUCCESS:

        case CoreMessage::BSL_LOCKED:
            return "BSL is not yet unlocked with Bootloader unlock password command or After BSL unlock";
        case CoreMessage::BSL_PWD_ERR:
            return "incorrect password has been sent to unlock bootloader";
        case CoreMessage::BSL_MULTIPLE_PWD_ERR:
            return "incorrect password has been sent to unlock bootloader 3 times";
        case CoreMessage::UNKNOWN_CMD:
            return "command given to the BSL was not recognized as valid command";
        case CoreMessage::INVALID_MEM_RANGE:
            return "given memory range is invalid";
        case CoreMessage::INVALID_CMD:
            return "command given to BSL is known command but it is invalid at that time instant and cannot be processed";
        case CoreMessage::FACTORY_RESET_DISABLED:
            return "factory reset is disabled in the BCR configuration";
        case CoreMessage::FACTORY_RESET_PWD_ERR:
            return "incorrect or no password has been sent with factory reset command";
        case CoreMessage::READOUT_ERR:
            return "memory read out be disabled in BCR configuration";
        case CoreMessage::INV_ADDR_OR_LEN:
            return "start address or data length for the flash programming is not 8-byte aligned";
        case CoreMessage::INV_LEN_VERIFICATION:
            return "data size sent for standalone verification is less than 1KB";
        case CoreMessage::BSL_UART_UNDEFINED:
            return "UNDEFINED";
        default:
            return "default undefined";
        }
    }

    static speed_t BSLBaudToSerialBaud(Baudrate rate)
    {
        switch(rate) {
        case Baudrate::BSL_B4800:
            return B4800;
        case Baudrate::BSL_B9600:
            return B9600;
        case Baudrate::BSL_B19200:
            return B19200;
        case Baudrate::BSL_B38400:
            return B38400;
        case Baudrate::BSL_B57600:
            return B57600;
        case Baudrate::BSL_B115200:
            return B115200;
        case Baudrate::BSL_B1000000:
            return B1000000;
        case Baudrate::BSL_B2000000:
            return B2000000;
        case Baudrate::BSL_B3000000:
            return B3000000;
        default:
            return B9600;      
        }
    }
    
    static const char* DeviceInfoToString(struct _device_info device_info)
    {
        // TODO -> C++20 format maybe?
        return "";
    }

};