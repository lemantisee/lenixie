# project(lwjson)

# add_library(${PROJECT_NAME} STATIC
#     lwjson/lwjson.h lwjson/lwjson.c
#     lwjson/lwjson_debug.c
#     lwjson/lwjson_opt.h
#     lwjson/lwjson_stream.c
# )

target_sources(${PROJECT_NAME} PRIVATE
    lwjson.h lwjson.c
    lwjson_debug.c
    lwjson_opt.h
    lwjson_stream.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    LWJSON_IGNORE_USER_OPTS
)