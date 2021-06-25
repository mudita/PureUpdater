// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

enum e_ecoboot_update_code{
    EcobootSuccess = 0,
    EcobootFileError,
    EcobootChecksumError,
    EcobootMMCError,
    EcobootCleanupFail,
    EcobootGenericError,
    None,
};

typedef struct update_error_t{
    enum e_ecoboot_update_code result;
    int extended_err_code;
} update_error_t;

const char *error_str(const enum e_ecoboot_update_code code);
const char *error_ext_str(const struct update_error_t *handle);
