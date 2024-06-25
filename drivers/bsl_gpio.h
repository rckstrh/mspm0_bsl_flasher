/*
 * bsl_gpio.h
 *
 *  Created on: Dec 19, 2023
 *      Author: Jonas Rockstroh
 */
#include "stdint.h"

class BSL_GPIO {
    public:
        struct _gpio_def {
            uint8_t bank;
            uint8_t pin;
        };

        BSL_GPIO(int _verbose_level, _gpio_def _bsl={.bank=_GPIO_BSL_BANK_, .pin=_GPIO_BSL_PIN_}, _gpio_def _reset={.bank=_GPIO_RESET_BANK_, .pin=_GPIO_RESET_PIN_});
        bool hard_reset(uint16_t ms_reset_time=default_ms_reset_time);
        bool enter_bsl();

    private:
        bool set_pin(_gpio_def gpio, bool level);

        static constexpr uint16_t default_ms_reset_time = 10;
        static constexpr uint16_t ms_bsl_out_settle = 10;

        // at the moment done via cmake flags
        _gpio_def bsl_out;
        _gpio_def reset_out;

        int verbose_level = 0;
};