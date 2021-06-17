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

target_compile_options( ${CMAKE_PROJECT_NAME} PRIVATE -Wall -Wextra)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS  
    "updater/*.c*"
    "platform/*.c*"
)


target_sources( ${CMAKE_PROJECT_NAME} PRIVATE ${SRC_FILES} )
