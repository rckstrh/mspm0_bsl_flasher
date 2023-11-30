/*
 * bsl_tool.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include <string>
#include "bsl_protocol.h"

class BSLTool {
    public:
        BSLTool();
        ~BSLTool();
    private:

        struct _uart_conf {
            unsigned int baud;
            unsigned int data_width;
            unsigned int stop_bits;
            bool parity;
            std::string port;
        } uart_conf = {.baud=9600, .data_width=8, .stop_bits=1, .parity=false, .port=""};    
};