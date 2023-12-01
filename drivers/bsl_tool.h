/*
 * bsl_tool.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include <string>
#include "bsl_uart.h"

class BSLTool {
    public:
        BSLTool();
        ~BSLTool();
    private:
        BSL_UART* uart_wrapper = nullptr;
};