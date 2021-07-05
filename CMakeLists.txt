cmake_minimum_required(VERSION 3.20)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

if (NOT DEFINED PLATFORM)
    message(WARNING "using default RT1051 platform")
    set(PLATFORM RT1051)
endif()

macro(use_rt1051)
    message(STATUS "Select toolchain, linker and includes for platform rt1051")
    include(toolchain-arm-none-eabi)
    set(LINKER_SCRIPTS 
        ldscripts/libs.ld 
        ldscripts/memory.ld 
        ldscripts/sections.ld
    )
    include(rt1051)
endmacro()

project(PureUpdater LANGUAGES C CXX ASM)

message("Platform: ${PLATFORM}")

if(DEFINED PLATFORM AND ${PLATFORM} STREQUAL linux)
    add_custom_target(${CMAKE_PROJECT_NAME} DEPENDS test_backup)
else()
    use_rt1051()
    add_custom_target(${CMAKE_PROJECT_NAME} DEPENDS updater)
endif()

add_subdirectory( thirdparty )
add_subdirectory( platform )
add_subdirectory( hal )
add_subdirectory( updater/common )
if(DEFINED PLATFORM AND ${PLATFORM} STREQUAL linux)
    add_subdirectory( unittest )
else()
    add_subdirectory( updater )
    add_subdirectory( tests )
endif()
