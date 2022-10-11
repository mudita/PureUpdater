package = "lsyspaths"
version = "1.0-1"
source = {
   url = "..." -- We don't have one yet
}
description = {
    summary = "providing system paths",
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
        lsyspaths = "lsyspaths.lua"
    }
}
