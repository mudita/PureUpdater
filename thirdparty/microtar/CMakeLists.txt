
add_library(microtar STATIC)

target_sources(microtar
    PRIVATE
    src/src/microtar.c
)

target_include_directories(microtar
    PUBLIC
    include
    PRIVATE
    src/src
)
