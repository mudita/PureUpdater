
add_library(seatest STATIC)

target_sources(seatest
    PRIVATE
    src/src/seatest.c
)

target_include_directories(seatest
    PUBLIC
    include
    PRIVATE
    src/src
)
