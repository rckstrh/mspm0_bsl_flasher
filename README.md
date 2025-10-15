# MSPM0 BSL UART flashing utility

This tool is meant to flash the MSPM0 MCU firmware on a phyVERSO EVCS controller board via TIs BSL UART protocol. Some commands like `read_binary_version` still have hardcoded offsets, where the mspm0_bsl_flasher tool reads the firmware binaries meant for this EVCS controller board. So if you try to fork/base your own flashing tool on this, you will have to modify offsets/lengths for this in your flashing tool implementation as well as modify your FW linker script accordingly.

## Requirements

Depending on the way you configure your cmake build, you would need to have one of the following available on the target hardware for accessing RESET/BSL pins:

- libgpiod-tools v1.x.x

## How to configure - libgpiod(-tools) v1.x.x

Without setting any cmake options/variables, the mspm0_bsl_flasher would assume that you have libgpiod-tools v1.x.x installed on your target system and will use `gpioset` to set RESET and BSL pins accordingly to bring the MSPM0 MCU into BSL bootloader mode.

By default, it is assumed that the RESET is available on `gpiochip1 line 23` and BSL pin is on `gpiochip1 line 12`. You can change those values by respectively setting the following parameters:

```
-D_GPIO_BSL_BANK_=
-D_GPIO_BSL_PIN_=
-D_GPIO_RESET_BANK_=
-D_GPIO_RESET_PIN_=
```