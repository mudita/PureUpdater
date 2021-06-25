//
// Created by maciej on 23.06.2021.
//

#ifndef ERROR_H
#define ERROR_H

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

#endif //ERROR_H
