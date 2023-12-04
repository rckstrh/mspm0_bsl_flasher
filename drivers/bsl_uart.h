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
        std::tuple<BSL::AckType, BSL::CoreMessage> unlock_bootloader(const uint8_t* passwd = bootloader_default_pw);
        std::tuple<BSL::AckType, BSL::CoreMessage> readback_data(const uint32_t addr, const uint32_t readback_len, uint8_t *dst);
        void set_bsl_max_buff_size(uint32_t _bsl_max_buff_size);
        
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

        uint32_t bsl_max_buff_size;
};