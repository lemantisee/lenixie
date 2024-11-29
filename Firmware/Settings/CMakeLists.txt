target_sources(${PROJECT_NAME} PRIVATE
    Settings.h Settings.cpp
    SettingsData.h SettingsData.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)