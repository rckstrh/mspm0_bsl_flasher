#include <iostream>
#include "bsl_tool.h"

void print_usage();

int main(int argc, char** argv){
    printf("MSPM0 BSL UART Flashing utility\n\n");

    if(argc < 4) {
        print_usage();
        return -1;
    }

    const char* serial_path = argv[1];
    const char* file_path = argv[2];
    const char* mode = argv[3];

    bool use_gpio = (mode[0] == '1') ? true : false;
    bool status = false;


    auto b = BSLTool(serial_path, use_gpio);

    if(use_gpio) {
        printf("Entering BSL mode\n");
        status = b.enter_bsl();
        if(!status) {
            printf("Could not enter BSL mode. Stopping...\n");
            return status;
        }
    }

    printf("Using serial %s to flash %s\n\n", serial_path, file_path);

    status = b.flash_image(file_path);
    
    return status;
}

void print_usage()
{
    printf("Usage: MSPM0_bsl_flasher SERIAL pathToBinary ENTER_BSL\n\n");
    printf("SERIAL: path to serial port (e.g. /dev/ttyACM0)\n");
    printf("ENTER_BSL: 0 => assumes MSP already in BSL mode; 1 => pulls hardcoded BSL and nRST pins accordingly\n");
    printf("=>Example: MSPM0_bsl_flasher /dev/ttyACM0 /home/foo/bar.bin\n\n");
}


