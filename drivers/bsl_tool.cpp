/*
 * bsl_tool.cpp
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include "bsl_tool.h"

BSLTool::BSLTool()
{
    uart_wrapper = new BSL_UART("/dev/ttyACM0");
    printf(">> Connecting\n");
    auto resp = uart_wrapper->connect();
    printf("<< %s\n", BSL::AckTypeToString(resp));
};

BSLTool::~BSLTool() 
{
    if(uart_wrapper != nullptr)
        delete uart_wrapper;
};