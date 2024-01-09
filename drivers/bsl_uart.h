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
        BSL_UART(const char* _serial_port, int _verbose_level=0);
        ~BSL_UART();
        bool open_serial();
        BSL::AckType connect();
        std::tuple<BSL::AckType, BSL::_device_info> get_device_info();
        BSL::AckType start_application();
        std::tuple<BSL::AckType, BSL::CoreMessage> unlock_bootloader(const uint8_t* passwd = bootloader_default_pw);
        std::tuple<BSL::AckType, BSL::CoreMessage> readback_data(const uint32_t addr, const uint32_t readback_len, uint8_t *dst);
        std::tuple<BSL::AckType, BSL::CoreMessage, uint32_t> verify(const uint32_t addr, const uint32_t size);
        std::tuple<BSL::AckType, BSL::CoreMessage> program_data(const uint32_t addr, const uint8_t* program_data, size_t program_size);
        std::tuple<BSL::AckType, BSL::CoreMessage> mass_erase();
        void set_bsl_max_buff_size(uint32_t _bsl_max_buff_size);
        BSL::AckType change_baudrate(BSL::Baudrate rate);
        
    private:
        Serial* serial = nullptr;

        BSL::CoreMessage receive_core_message();

        static constexpr uint16_t MAX_PAYLOAD_SIZE = 128;

        // "cmd global" const stuff
        static constexpr uint8_t header_len = 3;
        static constexpr uint8_t crc_len = 4;
        static constexpr uint8_t cmd_len = 1;
        static constexpr uint8_t addr_len = 4;
        static constexpr uint8_t password_len = 32;

        static constexpr uint8_t bootloader_default_pw[password_len] = {
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };

        uint32_t bsl_max_buff_size = 0;

        int verbose_level = 0;
};