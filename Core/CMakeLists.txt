target_sources(${PROJECT_NAME} PRIVATE
    startup_stm32f103c8tx.s
    stm32f1xx_hal_msp.c
    syscalls.c
    sysmem.c
    system_stm32f1xx.c
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}
)