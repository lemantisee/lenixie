target_sources(${PROJECT_NAME} PRIVATE
    startup_stm32f103c8tx.s
    syscalls.c
    sysmem.c
    system_stm32f1xx.c
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
)