# Target

set(UPDATER ${CMAKE_PROJECT_NAME}_RT)
add_executable(${UPDATER})
target_compile_features(${UPDATER} PRIVATE
    c_std_11
)

target_compile_options(${UPDATER} PRIVATE -Wall -Wextra -Werror)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "**/*.c*"
    "*.c*"
)

# Source directories
target_sources(
    ${UPDATER}
    PRIVATE
    ${SRC_FILES}
    )

# Libraries
target_link_libraries(
    ${UPDATER}
    PRIVATE 
    system
    system-runtime-objs 
    hal
    microtar
    klib
    common
    cjson
    md5
)

target_include_directories(
    ${UPDATER}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

include(secure-boot)

# Create binary format
print_section_sizes(${UPDATER})

if (ENABLE_SECURE_BOOT)
    create_binary_output(${UPDATER} "srec")
    add_custom_command(
        COMMENT "Generate signed {${UPDATER}}.bin (Secure Boot)"
        OUTPUT ${BIN_FILE}
        DEPENDS ${SREC_FILE}
        COMMAND ${CMAKE_SOURCE_DIR}/cmake/config/elftosb_wrapper.sh "${ELFTOSB_PATH}" "${CST_PATH}" -f imx -V
        -c ${CMAKE_BINARY_DIR}/imx_authenticated_hab.bd
        -o ${BIN_FILE}
        ${SREC_FILE}
        VERBATIM
    )
else()
    create_binary_output( ${UPDATER} "bin" ALL)
    add_custom_target( updater DEPENDS ${UPDATER}.bin)
endif()
