target_sources(${PROJECT_NAME} PRIVATE
    PanelClient.h PanelClient.cpp
    PanelMessage.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)