cmake_minimum_required(VERSION 3.20)

project(lenixie C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
 
add_executable(${PROJECT_NAME})


add_subdirectory(Core)
add_subdirectory(Libs)
add_subdirectory(src)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries( ${PROJECT_NAME} PRIVATE
    CMSIS
    CustomHID
    HAL
    Logger
    Json
    SString
)

target_compile_definitions( ${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:DEBUG>"
)

target_link_options( ${PROJECT_NAME} PRIVATE
    -Wl,--print-memory-usage
    -T${PROJECT_SOURCE_DIR}/STM32F103C8TX_FLASH.ld
)

target_compile_options( ${PROJECT_NAME} PRIVATE
    "$<$<CONFIG:Debug>:-g3>"
    "$<$<NOT:$<CONFIG:Debug>>:-g0>"
    "$<$<NOT:$<CONFIG:Debug>>:-Os>"
    -Wno-volatile #ignore -Wvolatile warning in c++20
    -Werror=return-type
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${PROJECT_NAME}>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
    $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.hex
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
    $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}.bin
)