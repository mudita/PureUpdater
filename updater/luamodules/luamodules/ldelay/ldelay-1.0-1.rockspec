package = "ldelay"
version = "1.0-1"
source = {
   url = "..." -- We don't have one yet
}
description = {
    summary = "delay-related utils",
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
        ldelay = "ldelay.lua"
    }
}
