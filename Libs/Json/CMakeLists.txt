project(Json)

add_library(${PROJECT_NAME} STATIC
    JsonObject.h JsonObject.cpp
)

add_subdirectory(lwjson)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    SString
)

if(BUILD_STM32)
    target_compile_options( ${PROJECT_NAME} PUBLIC
        -mcpu=cortex-m3
    )

    target_link_options( ${PROJECT_NAME} PUBLIC
        -mcpu=cortex-m3
    )
endif(BUILD_STM32)
