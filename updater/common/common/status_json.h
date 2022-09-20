#pragma once

#include <hal/boot_reason.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

enum status_json_result_e {
    OPERATION_SUCCESS,
    OPERATION_FAILURE
};

struct status_json_s {
    const char* file_path;
    const char* updater_version;
    const char* performed_operation;
    enum status_json_result_e operation_result;
};

bool status_json_save(struct status_json_s* status);

const char* status_json_boot_reason_to_operation_str(enum system_boot_reason_code code);

#ifdef __cplusplus
}
#endif
