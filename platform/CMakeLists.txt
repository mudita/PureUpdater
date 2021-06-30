# System library for all targets
set( LIB_NAME system )

add_library( ${LIB_NAME} STATIC )

set(LIB_RUNTIME_OBJS ${LIB_NAME}-runtime-objs)
add_library( ${LIB_RUNTIME_OBJS} OBJECT)
target_link_libraries( ${LIB_RUNTIME_OBJS} PUBLIC ${LIB_NAME} )


file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "*.c*"
)

target_sources( ${LIB_NAME} PRIVATE ${SRC_FILES} )
target_link_libraries( ${LIB_NAME} PUBLIC hal )

file(GLOB_RECURSE SRC_RUNTIME CONFIGURE_DEPENDS
    "syscalls/*.c*"
)
set( SRC_RUNTIME ${SRC_RUNTIME}
    "boot/startup_mimxrt1051.c"
    "boot/system_MIMXRT1051.c"
    "boot/evkbimxrt1050_sdram_ini_dcd.c"
    "boot/evkbimxrt1050_flexspi_nor_config.c"
)

target_sources( ${LIB_RUNTIME_OBJS} PRIVATE ${SRC_RUNTIME} )


# Include directories
target_include_directories( ${LIB_NAME} PUBLIC
    CMSIS
    include
    include/drivers
)
# User defines
target_compile_definitions(${LIB_NAME} PUBLIC
    DEBUG_CONSOLE_TRANSFER_NON_BLOCKING
    FSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1
    __STARTUP_INITIALIZE_NONCACHEDATA
    XIP_EXTERNAL_FLASH=0
    CPU_MIMXRT1051DVL6B_cm7
    CPU_MIMXRT1051DVL6B
    SDK_DEBUGCONSOLE=0
    __MCUXPRESSO
    __USE_CMSIS
    __NEWLIB__
    __FPU_PRESENT
    SKIP_SYSCLK_INIT
    _HAVE_SQLITE_CONFIG_H
    _GNU_SOURCE
    FSL_RTOS_BM
    SDK_OS_BAREMETAL
    CR_INTEGER_PRINTF
    PRINTF_FLOAT_ENABLE=0
    TESTS=0
    SDRAM_MPU_INIT
    IM_DEBUG=1
    DISABLE_WATCHDOG
)

if (NOT ENABLE_SECURE_BOOT)
    # Only enable the build-time boot header in a non-secure configuration,
    # a header for Secure Boot is added post-build when signing the binary
    target_compile_definitions(${LIB_NAME} PUBLIC
        XIP_BOOT_HEADER_ENABLE=1
        XIP_BOOT_HEADER_DCD_ENABLE=0
    )
endif()



#target_compile_options(
#    ${LIB_RUNTIME_OBJS} PRIVATE "-Werror"
#)
