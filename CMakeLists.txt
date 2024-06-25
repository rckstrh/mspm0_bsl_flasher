cmake_minimum_required(VERSION 3.0.0)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 3)
set(VERSION_PATCH 2)

project(MSPM0_bsl_flasher VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH} LANGUAGES C CXX)

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

include(CTest)
enable_testing()

include_directories(drivers)
include_directories(prog)

find_package( Boost REQUIRED COMPONENTS program_options )
include_directories( ${Boost_INCLUDE_DIRS} )

add_executable(MSPM0_bsl_flasher main.cpp drivers/bsl_tool.cpp drivers/serial.cpp drivers/bsl_uart.cpp drivers/bsl_gpio.cpp)

target_link_libraries(MSPM0_bsl_flasher Boost::program_options)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
