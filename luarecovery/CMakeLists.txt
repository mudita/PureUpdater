add_library(luarecovery STATIC)

target_sources(luarecovery PRIVATE
        src/common.c
        src/lrecovery.c
        src/lgui.c
        src/lsys.c
        src/lkeyboard.c
        src/lbootctrl.c
        )

target_include_directories(luarecovery PUBLIC include PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(luarecovery PRIVATE lua hal-common pure-core recovery)

# Define where LUA should search for the external modules:
# /user/tmp/lua/share
# /user/tmp/lua
target_compile_definitions(lua
        PRIVATE
        LUA_PATH_DEFAULT="/user/tmp/lua/share/?.lua\\\;?/init.lua\\\;/user/tmp/lua/?.lua\\\;/user/tmp/lua/?/init.lua"
        LUA_CPATH_DEFAULT=""
        )
# 'luafilesystem' library needs custom syscalls definitions
target_link_libraries(luafilesystem PRIVATE system)