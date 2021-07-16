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
   create_signed_binary( ${UPDATER} ALL)
else()
   create_binary_output( ${UPDATER} "bin" ALL)
endif()
