cmake_minimum_required(VERSION 3.0.0)
project(MSPM0_bsl_flasher VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

include_directories(drivers)
include_directories(prog)

add_executable(MSPM0_bsl_flasher main.cpp drivers/bsl_tool.cpp drivers/serial.cpp drivers/bsl_uart.cpp drivers/bsl_gpio.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
