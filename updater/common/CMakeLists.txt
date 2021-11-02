
add_library(common STATIC)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
    "common/*.c"
)

target_sources( common PRIVATE ${SRC_FILES} )
target_link_libraries(common microtar klib cjson images)

target_include_directories(microtar
    PUBLIC
    ./
    ./../../hal/src/display/
    PRIVATE
)
