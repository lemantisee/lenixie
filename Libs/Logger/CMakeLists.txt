project(Logger)

add_library(${PROJECT_NAME} STATIC
    Logger.h Logger.cpp
    RingBuffer.h
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC SString)