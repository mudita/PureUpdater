add_library(gui STATIC)

target_sources(gui
    PRIVATE
        gui.c
    PUBLIC
        include/gui/images
)

target_include_directories(gui
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(gui
        PRIVATE
        hal-common
)
