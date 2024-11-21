cmake_minimum_required(VERSION 3.20)

project(lenixie C CXX ASM)

set(CMAKE_CXX_STANDARD 20)

enable_testing()

add_subdirectory(Libs)

if(${BUILD_STM32})
    message("Building firmware")
    add_subdirectory(Firmware)
else()
    message("Building tests")
    add_subdirectory(Tests)
endif(${BUILD_STM32})