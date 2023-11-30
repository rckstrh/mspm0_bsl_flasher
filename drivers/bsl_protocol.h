/*
 * bsl_protocol.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include "stdint.h"

namespace BSL {

    /* packet format */
    struct _packet_format {
        uint8_t header;         // peripheral interface code
        uint8_t length[2];      // peripheral interface code
        uint8_t* bsl_core_data; // BSL core data
        uint8_t crc[4];         // peripheral interface code
    };

    /* types of BSL acknowledgements */
    enum class AckType {
        BSL_ACK                         = 0x00,
        BSL_ERROR_HEADER_INCORRECT      = 0x51,
        BSL_ERROR_CHECKSUM_INCORRECT    = 0x52,
        BSL_ERROR_PACKET_SIZE_ZERO      = 0x53,
        BSL_ERROR_PACKET_SIZE_TOO_BIG   = 0x54,
        BSL_ERROR_UNKNOWN_ERROR         = 0x55,
        BSL_ERROR_UNKNOWN_BAUD_RATE     = 0x56
    };

    static constexpr uint32_t crc32_initial_seed = 0xFFFFFFFF;

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

    enum class CoreRsp {
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
        uint8_t cmd_interpreter_version[2];
        uint8_t build_id[2];
        uint8_t app_version[4];
        uint8_t plugin_if_version[2];
        uint8_t bsl_max_buff_size[2];
        uint8_t bsl_buff_start_addr[4];
        uint8_t bcr_conf_id[4];
        uint8_t bsl_conf_id[4];
    };
};