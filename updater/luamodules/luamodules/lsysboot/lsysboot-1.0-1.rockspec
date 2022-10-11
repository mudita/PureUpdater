package = "lsysboot"
version = "1.0-1"
source = {
   url = "..." -- We don't have one yet
}
description = {
    summary = "system boot functionality",
    detailed = [[
    ]],
    license = "MIT"
}
dependencies = {
    "lua >= 5.1, < 5.5"
}
build = {
    type = "builtin",
    modules = {
        lsysboot = "lsysboot.lua"
    }
}
