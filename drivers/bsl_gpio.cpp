/*
 * bsl_gpio.cpp
 *
 *  Created on: Dec 19, 2023
 *      Author: Jonas Rockstroh
 */

#include "bsl_gpio.h"
#include <chrono>
#include <thread>
#include <iostream>

// LIBGPIOD1/gpioset based
#if GPIO_PROVIDER == GPIO_PROVIDER_LIBGPIOD1
bool BSL_GPIO::set_pin_gpioset(const _gpio_def gpio, bool level)
{
    char* cmd;
    int size = asprintf(&cmd, "gpioset %d %d=%d", gpio.bank, gpio.pin, (level ? 1 : 0));
    if((size == -1) || (!cmd)) {
        return false;
    }

    // debug printfs
    if(verbose_level > 2) {
        printf("%s\n", cmd);
    }

    int status = system(cmd);
    free(cmd);

    if(status == 0) {
        return true;
    } else {
        return false;
    }
}

bool BSL_GPIO::hard_reset()
{
    bool status = set_pin_gpioset(reset_out, true);
    if(!status) {
        printf("Could set reset active\n");
        return status;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(ms_reset_time));

    status = set_pin_gpioset(reset_out, false);
    if(!status) {
        printf("Could set reset inactive\n");
        return status;
    }

    return status;
}

bool BSL_GPIO::enter_bsl()
{
    bool status = set_pin_gpioset(bsl_out, true);
    if(!status) {
        printf("Could not set BSL pin high\n");
        return status;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(ms_bsl_out_settle));
    status = hard_reset();
    if(!status) {
        printf("Could not reset\n");
        return status;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_bsl_out_settle));
    status = set_pin_gpioset(bsl_out, false);
    if(!status) {
        printf("Could not set BSL pin low\n");
        return status;
    }

    return status;
}
#elif GPIO_PROVIDER == GPIO_PROVIDER_LIBGPIOD2
#include <gpiod.hpp>

bool BSL_GPIO::hard_reset()
{
    auto reset_request = gpiod::chip(std::string("/dev/gpiochip").append(std::to_string(reset_out.bank))).prepare_request()
			.set_consumer("BSL_GPIO::hard_reset()")
			.add_line_settings(
				reset_out.pin,
				::gpiod::line_settings().set_direction(
					::gpiod::line::direction::OUTPUT))
			.do_request();
    
    if(verbose_level > 2)
        printf("Setting reset_pin[gpiochip%d, line%d] to ACTIVE\n", reset_out.bank, reset_out.pin);
    reset_request.set_value(reset_out.pin, ::gpiod::line::value::ACTIVE);

    if(verbose_level > 2)
        printf("Waiting %d ms\n", ms_reset_time);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_reset_time));

    if(verbose_level > 2)
        printf("Setting reset_pin[gpiochip%d, line%d] to INACTIVE\n", reset_out.bank, reset_out.pin);
    reset_request.set_value(reset_out.pin, ::gpiod::line::value::INACTIVE);

    if(verbose_level > 2)
        printf("Releasing reset_pin[gpiochip%d, line%d]\n", reset_out.bank, reset_out.pin);
    reset_request.release();

    return true;
}

bool BSL_GPIO::enter_bsl()
{
    auto bsl_request = gpiod::chip(std::string("/dev/gpiochip").append(std::to_string(bsl_out.bank))).prepare_request()
			.set_consumer("BSL_GPIO::enter_bsl()")
			.add_line_settings(
				bsl_out.pin,
				::gpiod::line_settings().set_direction(
					::gpiod::line::direction::OUTPUT))
			.do_request();

    if(verbose_level > 2)
        printf("Setting bsl_pin[gpiochip%d, line%d] to ACTIVE\n", bsl_out.bank, bsl_out.pin);
    bsl_request.set_value(bsl_out.pin, ::gpiod::line::value::ACTIVE);

    if(verbose_level > 2)
        printf("Waiting %d ms\n", ms_bsl_out_settle);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_bsl_out_settle));

    if(verbose_level > 2)
        printf("Hard resetting\n");
    hard_reset();

    if(verbose_level > 2)
        printf("Waiting %d ms\n", ms_bsl_out_settle);
    std::this_thread::sleep_for(std::chrono::milliseconds(ms_bsl_out_settle));

    if(verbose_level > 2)
        printf("Setting bsl_pin[gpiochip%d, line%d] to INACTIVE\n", bsl_out.bank, bsl_out.pin);
    bsl_request.set_value(bsl_out.pin, ::gpiod::line::value::INACTIVE);

    if(verbose_level > 2)
        printf("Releasing bsl_pin[gpiochip%d, line%d]\n", bsl_out.bank, bsl_out.pin);
    bsl_request.release();

    return true;
}
#elif GPIO_PROVIDER == GPIO_PROVIDER_SYSFS
// TODO
#endif