/*
 * bsl_tool.h
 *
 *  Created on: Nov 30, 2023
 *      Author: Jonas Rockstroh
 */

#include <string>
//#include <vector>
#include "bsl_uart.h"
#include "bsl_gpio.h"

class BSLTool {
    public:
        BSLTool(const char* serial_port, bool use_gpio);
        ~BSLTool();

        // GPIO
        bool enter_bsl();

        // UART
        bool connect(bool force = false);
        bool change_baud(BSL::Baudrate baud);
        bool get_device_info();
        bool unlock();
        bool mass_erase();
        bool program_data(uint8_t* data, uint32_t load_addr, uint32_t size);
        bool verify(uint8_t *data, uint32_t load_addr, uint32_t size, uint32_t offset=0x8);
        bool start_application();

        //bool read_file(const char* path, std::vector<uint8_t> &buffer);
        bool open_file(const char* path, uint32_t &size);
        uint32_t read_file(uint8_t *dst, uint32_t size);
        bool close_file();
        bool flash_image(const char* filepath);
    private:
        BSL_UART* uart_wrapper = nullptr;
        FILE* input_file_handle = nullptr;
        BSL_GPIO* gpio_wrapper = nullptr;

        bool isConnected = false;
        bool isUnlocked = false;
        bool isErased = false;
        bool isProgrammed = false;
        bool isVerified = false;
        bool isStarted = false;
};