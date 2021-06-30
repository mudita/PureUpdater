# FAT filesystem library

add_library(fatfs)

target_sources(
    fatfs
    PRIVATE
    src/source/ff.c
    src/source/ffunicode.c
)
target_include_directories(
    fatfs
    PRIVATE
    src/source
    PUBLIC
    include
)

target_link_libraries(
    fatfs
    PRIVATE
    hal
)