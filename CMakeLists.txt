cmake_minimum_required(VERSION 3.5.0)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 4)
set(VERSION_PATCH 0)

project(MSPM0_bsl_flasher VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} LANGUAGES C CXX)

# GPIO chip/bank + line variables for non-labeled access
set(GPIO_BSL_BANK 1 CACHE STRING "BSL GPIO bank number")
set(GPIO_BSL_PIN 12 CACHE STRING "BSL GPIO pin number")
set(GPIO_RESET_BANK 1 CACHE STRING "Reset GPIO bank number")
set(GPIO_RESET_PIN 23 CACHE STRING "Reset GPIO pin number")

# Version variables
ADD_DEFINITIONS( -D_PROJECT_NAME_=\"${PROJECT_NAME}\")
ADD_DEFINITIONS( -D_VERSION_MAJOR_=\"${VERSION_MAJOR}\")
ADD_DEFINITIONS( -D_VERSION_MINOR_=\"${VERSION_MINOR}\")
ADD_DEFINITIONS( -D_VERSION_PATCH_=\"${VERSION_PATCH}\")

# map CMAKE variables to GPIO defines
ADD_DEFINITIONS( -D_GPIO_BSL_BANK_=${GPIO_BSL_BANK})
ADD_DEFINITIONS( -D_GPIO_BSL_PIN_=${GPIO_BSL_PIN})
ADD_DEFINITIONS( -D_GPIO_RESET_BANK_=${GPIO_RESET_BANK})
ADD_DEFINITIONS( -D_GPIO_RESET_PIN_=${GPIO_RESET_PIN})

# Set up the  user input variable and constrain to valid values
set(GPIO_PROVIDER "LIBGPIOD1" CACHE STRING "Which lib/subsystem provides GPIO access?")

set_property( CACHE GPIO_PROVIDER
    PROPERTY STRINGS LIBGPIOD1 LIBGPIOD2 SYSFS
)

# Convert the string to integer for preprocessor comparison
if ( "${GPIO_PROVIDER}" STREQUAL "LIBGPIOD1")
    set( GPIO_PROVIDER_VALUE 0 )
elseif( "${GPIO_PROVIDER}" STREQUAL "LIBGPIOD2" )
    set( GPIO_PROVIDER_VALUE 1 )
elseif( "${GPIO_PROVIDER}" STREQUAL "SYSFS" )
    set( GPIO_PROVIDER_VALUE 2 )
endif()

ADD_DEFINITIONS(
    -DGPIO_PROVIDER_LIBGPIOD1=0
    -DGPIO_PROVIDER_LIBGPIOD2=1
    -DGPIO_PROVIDER_SYSFS=2
    -DGPIO_PROVIDER=${GPIO_PROVIDER_VALUE})

include(CTest)
enable_testing()

include_directories(drivers)
include_directories(prog)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

add_executable(MSPM0_bsl_flasher
    main.cpp drivers/bsl_tool.cpp
    drivers/serial.cpp
    drivers/bsl_uart.cpp
    drivers/bsl_gpio.cpp
)

target_link_libraries(MSPM0_bsl_flasher Boost::program_options)

if( "${GPIO_PROVIDER}" STREQUAL "LIBGPIOD2" )
    # Find libgpiod
    find_library(GPIOD_LIBRARY gpiod)
    if(NOT GPIOD_LIBRARY)
        message(FATAL_ERROR "libgpiod not found")
    endif()
    target_link_libraries(MSPM0_bsl_flasher gpiodcxx)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
