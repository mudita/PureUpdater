add_library(images INTERFACE)

target_sources(images
    INTERFACE
        "include/ui_update_error.h"
        "include/ui_update_in_progress.h"
        "include/ui_update_success.h"
)

target_include_directories(images
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
