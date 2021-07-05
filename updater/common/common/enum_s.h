#pragma once

/// val ->`case $val: return "$val"`
#define ENUMS(val) case val: return "" #val ""
