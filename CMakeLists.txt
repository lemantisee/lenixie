cmake_minimum_required(VERSION 3.20)

project(lenixie LANGUAGES C CXX ASM VERSION 0.2.0)

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

configure_file(Version.h.in Version.h)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})