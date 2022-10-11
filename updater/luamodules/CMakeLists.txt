
add_library(luamodules STATIC)

target_sources(luamodules PRIVATE
        luamodules/lsysboot/lsysboot.c
        luamodules/lsyspaths/lsyspaths.c
        )

target_include_directories(luamodules PUBLIC .)
target_link_libraries(luamodules PUBLIC lua hal-common)