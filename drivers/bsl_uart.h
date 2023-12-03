/*
 * bsl_uart.h
 *
 *  Created on: Dec 1, 2023
 *      Author: Jonas Rockstroh
 */

#include "serial.h"
#include "bsl_protocol.h"

class BSL_UART {
    public:
        BSL_UART(const char* _serial_port);
        ~BSL_UART();

        BSL::AckType connect();
        std::tuple<BSL::AckType, BSL::_device_info> get_device_info();
        BSL::AckType start_application();
        std::tuple<BSL::AckType, BSL::RspMsg> unlock_bootloader(const uint8_t* passwd = bootloader_default_pw);
        
    private:
        Serial* serial = nullptr;

        // "cmd global" const stuff
        static constexpr uint8_t header_len = 3;
        static constexpr uint8_t crc_len = 4;
        static constexpr uint8_t password_len = 32;

        static constexpr uint8_t bootloader_default_pw[password_len] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
};