#! /bin/bash
# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

# This wrapper script expects filepaths to elftosb and cst NXP utilities
# as the first and second pos. arg. respectively. It passes the rest of
# pos. args to elftosb. Its main purpose is to export the path to cst utility
# so it is visible to elftosb.

if [[ $# -lt 2 ]]; then
    exit 1
fi

elftosb_path=$1
shift 1
cst_path=$1
shift 1

if [[ -n "$elftosb_path" ]]; then
    elftosb_exec="$elftosb_path/elftosb"
else
    elftosb_exec="elftosb"
fi

if [[ -n "$cst_path" ]]; then
    # Export CST path for elftosb (assume already in PATH if empty)
    export "PATH=$cst_path:$PATH"
fi

# Run elftosb passing the remaining args
"$elftosb_exec" $@
