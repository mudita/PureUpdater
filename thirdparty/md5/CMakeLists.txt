
add_library(md5 STATIC)

target_sources(md5
    PRIVATE
        src/md5/md5.c
)

target_include_directories(md5
    PUBLIC
    include
    PRIVATE
    src/md5
)
