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
        BSLTool(const char* serial_port, bool use_gpio, int _verbose_level=0);
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

        bool open_file(const char* path, uint32_t &size);
        uint32_t read_file(uint8_t *dst, uint32_t size);
        bool close_file();
        std::string read_file_version(uint32_t offset=0x000000c0, uint32_t fw_version_len=51);

        bool flash_image(const char* filepath, bool force);
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

        int verbose_level = 0;
};