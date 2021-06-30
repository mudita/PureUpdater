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
    c_std_11
)

add_compile_options( "-Werror" )


target_compile_options( ${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused)
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "updater/*.c*"
)


# Source directories
target_sources( ${CMAKE_PROJECT_NAME} PRIVATE ${SRC_FILES} )

# Libraries
target_link_libraries( ${CMAKE_PROJECT_NAME} 
    PRIVATE 
    system
    system-runtime-objs 
    hal
)


include(secure-boot)

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
add_subdirectory( platform )
add_subdirectory( hal )
add_subdirectory( tests )