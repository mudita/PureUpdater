# Unit tests on the target platform file

set( TARGET_TEST ${CMAKE_PROJECT_NAME}-test )
add_executable( ${TARGET_TEST} EXCLUDE_FROM_ALL)

target_compile_options( ${TARGET_TEST} PRIVATE -Wall -Wextra -Wno-unused)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "src/*.c*"
)

# Source directories
target_sources( ${TARGET_TEST} PRIVATE ${SRC_FILES} )

target_compile_features(${TARGET_TEST} PRIVATE
    c_std_11
)
target_compile_options( ${TARGET_TEST} PRIVATE 
    -Wall -Wextra
)

target_link_libraries( ${TARGET_TEST} 
    PRIVATE 
    system
    system-runtime-objs 
    hal
    seatest
)
# Create binary format
print_section_sizes( ${TARGET_TEST})


if (ENABLE_SECURE_BOOT)
    create_binary_output( ${TARGET_TEST} "srec")
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
    create_binary_output( ${TARGET_TEST} "bin")
    add_custom_target( tests  DEPENDS ${TARGET_TEST}.bin)
endif()