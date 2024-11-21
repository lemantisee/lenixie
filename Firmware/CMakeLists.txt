add_executable(${PROJECT_NAME}
    main.cpp
    MonitorCommand.h
    Uart.h Uart.cpp

    Clock/DateTime.h Clock/DateTime.cpp
    Clock/NTPRequest.h Clock/NTPRequest.cpp
    Clock/RTClock.h Clock/RTClock.cpp

    ESP8266/ESP8266.h ESP8266/ESP8266.cpp
    ESP8266/EspAtCommand.h ESP8266/EspAtCommand.cpp

    Indicator/BCDDecoder.h Indicator/BCDDecoder.cpp
    Indicator/DynamicIndication.h Indicator/DynamicIndication.cpp

    Usb/CustomHid.h Usb/CustomHid.cpp
    Usb/UsbDriverF103.h Usb/UsbDriverF103.cpp
    Usb/UsbDeviceDescriptor.h Usb/UsbDeviceDescriptor.cpp
    Usb/UsbDevice.h Usb/UsbDevice.cpp
)

add_subdirectory(Core)
add_subdirectory(PanelClient)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Clock
    ${CMAKE_CURRENT_SOURCE_DIR}/ESP8266
    ${CMAKE_CURRENT_SOURCE_DIR}/Indicator
    ${CMAKE_CURRENT_SOURCE_DIR}/Misc
    ${CMAKE_CURRENT_SOURCE_DIR}/SMProtocol
    ${CMAKE_CURRENT_SOURCE_DIR}/Usb
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