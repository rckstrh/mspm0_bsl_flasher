/*
 * bsl_tool.cpp
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include "bsl_tool.h"
#include "testprog.h"
#include <chrono>
#include <thread>


BSLTool::BSLTool()
{
    uart_wrapper = new BSL_UART("/dev/ttyACM0");

    // connect
    {
        printf(">> Connecting\n");
        auto resp = uart_wrapper->connect();
        printf("<< %s\n", BSL::AckTypeToString(resp));
        if(resp != BSL::AckType::BSL_ACK) {
            printf("Could not connect. Stopping...\n");
            return;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // change baud in BSL
    {
        printf(">> Changing baudrate to 115200\n");
        auto resp = uart_wrapper->change_baudrate(BSL::Baudrate::BSL_B115200);
        printf("<< %s\n", BSL::AckTypeToString(resp));
        if(resp != BSL::AckType::BSL_ACK) {
            printf("Could not change baudrate. Stopping...\n");
            //return;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    

    // get device info
    {
        printf(">> Getting device info\n");
        const auto [ack, device_info] = uart_wrapper->get_device_info();
        printf("<< %s\n", BSL::AckTypeToString(ack));
        if(ack != BSL::AckType::BSL_ACK) {
            printf("Could not get device info. Stopping...\n");
            return;
        }

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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // unlock bootloader
    {
        printf(">> Unlocking bootloader\n");
        const auto [ack, msg] = uart_wrapper->unlock_bootloader();
        printf("<< %s\n", BSL::AckTypeToString(ack));

        if(ack != BSL::AckType::BSL_ACK) {
            printf("Could not unlock bootloader. Please check configured password. Stopping...\n");
            return;
        }
    }

    // mass erase, program data may be faulty otherwise
    {
        printf(">> Mass erase before programming\n");
        const auto [ack, msg] = uart_wrapper->mass_erase();
        printf("<< %s\n", BSL::AckTypeToString(ack));
    }

    // write blinky
    {
        constexpr uint32_t addr =  0x0;
        uint32_t size =  blinky_workspace_v12_blink_led_LP_MSPM0G3507_freertos_ticlang_Debug_blink_led_LP_MSPM0G3507_freertos_ticlang_bin_len;
        printf(">> Program data @0x%08x, size=%d bytes\n", addr, size);
        const auto [ack, msg] = uart_wrapper->program_data(addr, blinky_workspace_v12_blink_led_LP_MSPM0G3507_freertos_ticlang_Debug_blink_led_LP_MSPM0G3507_freertos_ticlang_bin, size);
        printf("<< ACK: %s MSG: %s\n", BSL::AckTypeToString(ack), BSL::CoreMessageToString(msg));
    }

    // verify/standalone CRC blinky
    {
        const uint32_t block_size = 1024;
        constexpr uint32_t addr = 0x00;
        printf(">> Standalone verification @0x%08x, size=%dbytes\n", addr, block_size);
        const auto [ack, msg, crc] = uart_wrapper->verify(addr, block_size);
        printf("<< ACK: %s MSG: %s CRC: 0x%08x\n", BSL::AckTypeToString(ack), BSL::CoreMessageToString(msg), crc);

        // do CRC over input image
        uint8_t *prog_data = blinky_workspace_v12_blink_led_LP_MSPM0G3507_freertos_ticlang_Debug_blink_led_LP_MSPM0G3507_freertos_ticlang_bin;
        uint32_t prog_size = 1024;
        auto prog_crc = BSL::softwareCRC(prog_data, prog_size);
        printf("Prog CRC: 0x%08x\n", prog_crc);
    }

    // start application
    {
        printf(">> Starting application\n");
        auto resp = uart_wrapper->start_application();
        printf("<< %s\n", BSL::AckTypeToString(resp));
    }
};

BSLTool::~BSLTool() 
{
    if(uart_wrapper != nullptr)
        delete uart_wrapper;
};