cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

include(toolchain-arm-none-eabi)
project(PureUpdater LANGUAGES C CXX ASM)

set(LINKER_SCRIPTS 
    ldscripts/libs.ld 
    ldscripts/memory.ld 
    ldscripts/sections.ld
)
include(rt1051)


add_executable(${CMAKE_PROJECT_NAME})

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE
    cxx_std_17
    cxx_noexcept
    c_std_11
)

target_compile_options( ${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "updater/*.c*"
    "platform/*.c*"
)


# Source directories
target_sources( ${CMAKE_PROJECT_NAME} PRIVATE ${SRC_FILES} )

#Libraries
target_link_libraries( ${CMAKE_PROJECT_NAME} PRIVATE
    fatfs
)

# Include directories
target_include_directories( ${CMAKE_PROJECT_NAME} PRIVATE
    platform/CMSIS
    platform/include
    platform/include/drivers
)
# User defines
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
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

target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE "-Werror"
)

include(secure-boot)

if (NOT ENABLE_SECURE_BOOT)
    # Only enable the build-time boot header in a non-secure configuration,
    # a header for Secure Boot is added post-build when signing the binary
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
        XIP_BOOT_HEADER_ENABLE=1
        XIP_BOOT_HEADER_DCD_ENABLE=0
    )
endif()

# Create binary format
print_section_sizes( ${CMAKE_PROJECT_NAME})


if (ENABLE_SECURE_BOOT)
    create_binary_output( ${CMAKE_PROJECT_NAME} "srec")
    add_custom_command(
        COMMENT "Generate signed {$CMAKE_PROJECT_NAME}.bin (Secure Boot)"
        OUTPUT ${BIN_FILE}
        DEPENDS ${SREC_FILE}
        COMMAND ${CMAKE_SOURCE_DIR}/cmake/config/elftosb_wrapper.sh "${ELFTOSB_PATH}" "${CST_PATH}" -f imx -V
        -c ${CMAKE_BINARY_DIR}/imx_authenticated_hab.bd
        -o ${BIN_FILE}
        ${SREC_FILE}
        VERBATIM
    )
else()
    create_binary_output( ${CMAKE_PROJECT_NAME} "bin")
endif()

add_subdirectory( thirdparty )