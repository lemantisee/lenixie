target_sources(${PROJECT_NAME} PRIVATE
    MessageSession.h
    DateTimeSession.h DateTimeSession.cpp
    LogSession.h LogSession.cpp
    NetworkSession.h NetworkSession.cpp
    NtpSession.h NtpSession.cpp

    PanelClient.h PanelClient.cpp
    PanelMessage.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)