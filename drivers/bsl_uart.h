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
        
    private:
        Serial* serial = nullptr;

            // "cmd global" const stuff
        static constexpr uint8_t header_len = 3;
        static constexpr uint8_t crc_len = 4;


        // hardcoded atm in serial.cpp
        /*
        struct _uart_conf {
            unsigned int baud;
            unsigned int data_width;
            unsigned int stop_bits;
            bool parity;
            std::string port;
        } uart_conf = {.baud=9600, .data_width=8, .stop_bits=1, .parity=false, .port=""};    
        */
};