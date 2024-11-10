target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    MonitorCommand.h
    LogDump.h LogDump.cpp

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

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Clock
    ${CMAKE_CURRENT_SOURCE_DIR}/ESP8266
    ${CMAKE_CURRENT_SOURCE_DIR}/Indicator
    ${CMAKE_CURRENT_SOURCE_DIR}/Misc
    ${CMAKE_CURRENT_SOURCE_DIR}/SMProtocol
    ${CMAKE_CURRENT_SOURCE_DIR}/Usb
)