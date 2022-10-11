local sysboot = {}

-- Load dummy boot reason from a file. Its content can be easily filled during unit testing using automatic tools.
local fd = assert(io.open("sysboot.txt"))
local reason = fd:read("*a")
fd:close()

local reason_as_string = { [0] = "system_boot_reason_update",
                           [1] = "system_boot_reason_recovery",
                           [2] = "system_boot_reason_factory",
                           [3] = "system_boot_reason_pgm_keys"
}

local reason_unknown = "system_boot_reason_unknown"

function sysboot.reason()
    return tonumber(reason)
end

function sysboot.reason_str()
    local r = tonumber(reason)

    if (reason_as_string[r]) then
        return reason_as_string[r]
    else
        return reason_unknown
    end
end

function sysboot.reason_to_str(reason)
    if (reason_as_string[reason]) then
        return reason_as_string[reason]
    else
        return reason_unknown
    end
end

return sysboot