#include <iostream>
#include "bsl_tool.h"

void print_usage();

int main(int argc, char** argv){
    printf("MSPM0 BSL UART Flashing utility\n\n");

    if(argc < 3) {
        print_usage();
        return -1;
    }

    const char* serial_path = argv[1];
    const char* file_path = argv[2];

    printf("Using serial %s to flash %s\n\n", serial_path, file_path);

    auto b = BSLTool(serial_path);
    bool status = b.flash_image(file_path);

    return status;
}

void print_usage()
{
    printf("Usage: MSPM0_bsl_flasher SERIAL pathToBinary\n=>Example: MSPM0_bsl_flasher /dev/ttyACM0 /home/foo/bar.bin\n\n");
}
