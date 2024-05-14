/*
 * bsl_tool.cpp
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include "bsl_tool.h"
#include <chrono>
#include <thread>


BSLTool::BSLTool(const char* serial_port, bool use_gpio, int _verbose_level) : verbose_level(_verbose_level)
{
    if(serial_port != nullptr) {
        uart_wrapper = new BSL_UART(serial_port, verbose_level);
    }

    if(use_gpio) {
        gpio_wrapper = new BSL_GPIO(verbose_level);
    }
};

BSLTool::~BSLTool() 
{
    if(uart_wrapper != nullptr)
        delete uart_wrapper;
};

bool BSLTool::enter_bsl()
{
    if(gpio_wrapper != nullptr) {
        return gpio_wrapper->enter_bsl();
    } else {
        return false;
    }
}

bool BSLTool::connect(bool force)
{
    if(!force && isConnected) {
        printf("Already connected.");
        return isConnected;
    }

    printf(">> Connecting\n");
    auto resp = uart_wrapper->connect();

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(resp));
    }

    if(resp != BSL::AckType::BSL_ACK) {
        printf("Could not connect. Stopping...\n");
        isConnected = false;
        return isConnected;
    }

    isConnected = true;
    return isConnected;
}

bool BSLTool::change_baud(BSL::Baudrate baud)
{
    printf(">> Changing baudrate to 115200\n");
    auto resp = uart_wrapper->change_baudrate(baud);

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(resp));
    }

    if(resp != BSL::AckType::BSL_ACK) {
        printf("Could not change baudrate. Stopping...\n");
        return false;
    }
    return true;
}

bool BSLTool::get_device_info()
{
    printf(">> Getting device info\n");
    const auto [ack, device_info] = uart_wrapper->get_device_info();

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(ack));
    }

    if(ack != BSL::AckType::BSL_ACK) {
        printf("Could not get device info. Stopping...\n");
        return false;
    }

    if(verbose_level > 0) {
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

    return true;
}

bool BSLTool::unlock()
{
    printf(">> Unlocking bootloader\n");
    const auto [ack, msg] = uart_wrapper->unlock_bootloader();

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(ack));
    }

    if(ack != BSL::AckType::BSL_ACK) {
        printf("Could not unlock bootloader. Please check configured password. Stopping...\n");
        isUnlocked = false;
        return isUnlocked;
    }

    isUnlocked = true;
    return isUnlocked;
}

bool BSLTool::mass_erase()
{
    printf(">> Mass erase before programming\n");
    const auto [ack, msg] = uart_wrapper->mass_erase();

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(ack));
    }

    if(ack != BSL::AckType::BSL_ACK) {
        printf("Could not mass erase flash. Stopping...\n");
        isErased = false;
        return isErased;
    }

    isErased = true;
    return isErased;
}

bool BSLTool::program_data(uint8_t* data, uint32_t load_addr, uint32_t size)
{
    printf(">> Program data @0x%08x, size=%d bytes\n", load_addr, size);
    const auto [ack, msg] = uart_wrapper->program_data(load_addr, data, size);

    if(verbose_level > 1) {
        printf("<< ACK: %s MSG: %s\n", BSL::AckTypeToString(ack), BSL::CoreMessageToString(msg));
    }

    if((ack != BSL::AckType::BSL_ACK) || (msg != BSL::CoreMessage::SUCCESS)) {
        printf("Could not program. Stopping...\n");
        isProgrammed = false;
        return isProgrammed;
    }

    isProgrammed = true;
    return isProgrammed;
}

bool BSLTool::verify(uint8_t *data, uint32_t load_addr, uint32_t size, uint32_t offset)
{
    
    const uint32_t block_size = size-offset;
    const uint32_t addr = load_addr+offset;
    printf(">> Standalone verification");
    if(verbose_level > 1) {
        printf(" @0x%08x, size=%dbytes", addr, block_size);
    }
    printf("\n");

    const auto [ack, msg, mcu_crc] = uart_wrapper->verify(addr, block_size);

    if(verbose_level > 1) {
        printf("<< ACK: %s MSG: %s MCU_CRC: 0x%08x\n", BSL::AckTypeToString(ack), BSL::CoreMessageToString(msg), mcu_crc);
    }

    if(ack != BSL::AckType::BSL_ACK) {
        printf("Could not receive verification response. Stopping...\n");
        isVerified = false;
        return isVerified;
    }

    // do CRC over input image
    uint32_t prog_size = block_size;
    auto prog_crc = BSL::softwareCRC(data+offset, block_size);
    if(verbose_level > 2) {
        printf("Prog CRC: 0x%08x\n", prog_crc);
    }

    if(prog_crc != mcu_crc) {
        printf("CRC mismatch\n");
        isVerified = false;
        return isVerified;
    }

    printf(">> Verified programmed data\n");

    isVerified = true;
    return isVerified;
}

bool BSLTool::start_application()
{
    printf(">> Starting application\n");
    auto ack = uart_wrapper->start_application();

    if(verbose_level > 1) {
        printf("<< %s\n", BSL::AckTypeToString(ack));
    }

    if(ack != BSL::AckType::BSL_ACK) {
        printf("Could not start application. Stopping...\n");
        isStarted = false;
        return isStarted;
    }

    isStarted = true;
    return isStarted;
}

bool BSLTool::open_file(const char* path, uint32_t &size)
{
    close_file();

    input_file_handle = fopen(path, "rb");
    if(input_file_handle == nullptr) {
        printf("Read file: Can not open file %s\n", path);
        return false;
    }

    fseek(input_file_handle, 0L, SEEK_END);
    size = ftell(input_file_handle);
    fseek(input_file_handle, 0L, SEEK_SET);

    return true;
}

bool BSLTool::close_file()
{
    if(input_file_handle) {
        if(!fclose(input_file_handle))
            return false;
    }
    return true;
}

uint32_t BSLTool::read_file(uint8_t *dst, uint32_t size)
{
    uint32_t bytes_read = 0;
    bytes_read = fread(dst, size, 1, input_file_handle);
    return bytes_read;
}

std::string BSLTool::read_file_version(uint32_t offset, uint32_t fw_version_len)
{
    if(input_file_handle == nullptr) {
        printf("Open file first!\n");
        return "";
    }  

    uint32_t bytes_read = 0;
    char fw_version[fw_version_len] = "";

    fseek(input_file_handle, offset, SEEK_SET);
    bytes_read = fread(fw_version, 1, fw_version_len, input_file_handle);

    if(bytes_read != fw_version_len) {
        printf("Error reading fw version\n");
        return "";
    }

    return fw_version;
}

bool BSLTool::flash_image(const char* filepath, bool force)
{
    bool status = false;

    status = connect();
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    status = change_baud(BSL::Baudrate::BSL_B115200); 
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    status = get_device_info();
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    status = unlock();
    if(!status) {
        return false;
    }

    uint32_t size = 0;
    status = open_file(filepath, size);
    if(!status) {
        printf("Error opening file %s\n", filepath);
        return false;
    }

    uint8_t data[size] = {0};
    status = read_file(data, size);
    if(!status) {
        printf("Error reading file %s\n", filepath);
        return false;
    }

    if(!force) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        status = verify(data, 0x0, size);
        if(status) {
            printf("Already up-to-date");

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            status = start_application();
            if(status) {
                return true;
            }
        } else {
            printf("Updating");
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    status = mass_erase();
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    status = program_data(data, 0x0, size);
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    status = verify(data, 0x0, size);
    if(!status) {
        return false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    status = start_application();
    if(!status) {
        return false;
    }

    // debug print flag summary
    printf("\nStatus:\n\tProgrammed: %d\n\tVerified: %d\n\tStarted: %d\n", isProgrammed, isVerified, isStarted);

    return true;
}
