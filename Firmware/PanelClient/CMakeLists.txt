target_sources(${PROJECT_NAME} PRIVATE
    MessageSession.h
    DateTimeSession.h DateTimeSession.cpp
    LogSession.h LogSession.cpp

    PanelClient.h PanelClient.cpp
    PanelMessage.h
    PanelCommand.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)