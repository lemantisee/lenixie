target_sources(${PROJECT_NAME} PRIVATE
    main.cpp

    Clock/NTPRequest.h Clock/NTPRequest.cpp
    Clock/RTClock.h Clock/RTClock.cpp

    ESP8266/ESP8266.h ESP8266/ESP8266.cpp
    ESP8266/EspAtCommand.h ESP8266/EspAtCommand.cpp

    Indicator/BCDDecoder.h Indicator/BCDDecoder.cpp
    Indicator/DynamicIndication.h Indicator/DynamicIndication.cpp

    Misc/Logger.h Misc/Logger.cpp
    Misc/SString.h 
    Misc/StringBuffer.h

    SMProtocol/Interface.h
    SMProtocol/SMProtocol.h SMProtocol/SMProtocol.cpp
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Clock
    ${CMAKE_CURRENT_SOURCE_DIR}/ESP8266
    ${CMAKE_CURRENT_SOURCE_DIR}/Indicator
    ${CMAKE_CURRENT_SOURCE_DIR}/Misc
    ${CMAKE_CURRENT_SOURCE_DIR}/SMProtocol
)