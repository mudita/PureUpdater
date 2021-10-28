add_library(ui_screens STATIC)

target_sources(ui_screens
    PRIVATE
        ui_screens.c
    PUBLIC
        include/hal/system.h
)

target_include_directories(hal
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(hal
    PRIVATE
        hal-common
        system)
