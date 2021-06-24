
add_library(littlefs STATIC)

target_sources(littlefs
    PRIVATE
        src/lfs.c
        src/lfs_util.c
        src/bd/lfs_filebd.c
        src/bd/lfs_rambd.c
        src/bd/lfs_testbd.c
)


#target_compile_definitions(littlefs 
#    PUBLIC 
#        LFS_DEBUG
#)

target_include_directories(littlefs
    PUBLIC
    include
    PRIVATE
    src
)
