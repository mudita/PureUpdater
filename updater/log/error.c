//
// Created by maciej on 24.06.2021.
//
#include "error.h"

const char *error_str(const enum e_ecoboot_update_code code) {
    switch (code) {
        case EcobootSuccess:
            return "EcobootSuccess";
        case EcobootFileError:
            return "EcobootFileError";
        case EcobootChecksumError:
            return "EcobootChecksumError";
        case EcobootMMCError:
            return "EcobootMMCError";
        case EcobootCleanupFail:
            return "EcobootCleanupFail";
        case EcobootGenericError:
            return "EcobootGenericError";
    }
    return "UNDEFINED";
}

const char *error_ext_str(const struct update_error_t *handle) {
    return "UNDEFINED";
}
