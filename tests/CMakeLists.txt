# Unit tests on the target platform file

set( TARGET_TEST ${CMAKE_PROJECT_NAME}-test )
add_executable( ${TARGET_TEST} EXCLUDE_FROM_ALL)

target_compile_options( ${TARGET_TEST} PRIVATE -Wall -Wextra -Wno-unused)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "src/*.c*"
)

# Source directories
target_sources( ${TARGET_TEST} PRIVATE ${SRC_FILES} )

target_compile_features(${CMAKE_PROJECT_NAME} PRIVATE
    cxx_std_17
    cxx_noexcept
    c_std_11
)
