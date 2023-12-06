#include <iostream>
#include "bsl_tool.h"

int main(int, char**){
    printf("MSPM0 BSL UART Flashing utility\n\n");

    auto b = BSLTool("/dev/ttyACM0");
    //b.flash_image("/home/j/blinky_workspace_v12/blink_led_LP_MSPM0G3507_freertos_ticlang/Debug/blink_led_LP_MSPM0G3507_freertos_ticlang.bin");
    b.flash_image("/home/j/customers/Phytec/phyVERSO/phyverso-firmware/phyverso-firmware/Debug/phyverso-firmware.bin");
}
