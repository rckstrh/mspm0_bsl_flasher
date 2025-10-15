/*
 * bsl_gpio.h
 *
 *  Created on: Dec 19, 2023
 *      Author: Jonas Rockstroh
 */
#include "stdint.h"

class BSL_GPIO {
    public:
#if GPIO_PROVIDER == GPIO_PROVIDER_SYSFS
        struct _gpio_def {
            const char* path;
        }
#else
        struct _gpio_def {
            const uint8_t bank;
            const uint8_t pin;
        };
#endif
        BSL_GPIO(int _verbose_level) : verbose_level(_verbose_level) {};
        bool hard_reset();
        bool enter_bsl();

    private:
#if GPIO_PROVIDER == GPIO_PROVIDER_LIBGPIOD1
        bool set_pin_gpioset(const _gpio_def gpio, bool level);
#endif

        static constexpr uint16_t ms_reset_time = 10;
        static constexpr uint16_t ms_bsl_out_settle = 10;

        // at the moment done via cmake flags
        const struct _gpio_def bsl_out = {
            .bank = _GPIO_BSL_BANK_,
            .pin = _GPIO_BSL_PIN_,
        };

        const struct _gpio_def reset_out = {
            .bank = _GPIO_RESET_BANK_,
            .pin = _GPIO_RESET_PIN_,
        };

        int verbose_level = 0;
};