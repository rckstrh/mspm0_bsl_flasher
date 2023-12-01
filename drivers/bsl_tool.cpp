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
    // connect
    {
        printf(">> Connecting\n");
        auto resp = uart_wrapper->connect();
        printf("<< %s\n", BSL::AckTypeToString(resp));
    }
    // get device info
    {
        printf(">> Getting device info\n");
        const auto [ack, device_info] = uart_wrapper->get_device_info();
        printf("<< %s\n", BSL::AckTypeToString(ack));
        printf("<< Device Info: \n");
        printf("\tCommand interpreter version: 0x%x\n", device_info.cmd_interpreter_version);
        printf("\tBuild ID: 0x%x\n", device_info.build_id);
        printf("\tApplication version: 0x%x\n", device_info.app_version);
        printf("\tPlug-In interface version: 0x%x\n", device_info.plugin_if_version);
        printf("\tBSL max buffer size: 0x%x\n", device_info.bsl_max_buff_size);
        printf("\tBSL start address: 0x%x\n", device_info.bsl_buff_start_addr);
        printf("\tBCR conf ID: 0x%x\n", device_info.bcr_conf_id);
        printf("\tBSL conf ID: 0x%x\n", device_info.bsl_conf_id);
    }
};

BSLTool::~BSLTool() 
{
    if(uart_wrapper != nullptr)
        delete uart_wrapper;
};