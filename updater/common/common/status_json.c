#include "status_json.h"
#include <common/log.h>
#include <cJSON/cJSON.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

static void _autoclose(FILE** fp) {
    fclose(*fp);
}

static void _autodelete(cJSON** json) {
    cJSON_Delete(*json);
}

static void _autofree(char** mem) {
    free(*mem);
}

#define AUTOCLOSE(var) FILE* var __attribute__((__cleanup__(_autoclose)))
#define AUTODELETE(var) cJSON* var __attribute__((__cleanup__(_autodelete)))
#define AUTOFREE(var) char* var __attribute__((__cleanup__(_autofree)))

bool status_json_delete(struct status_json_s* status) {
    if(unlink(status->file_path)) {
        return false;
    }
    return true;
}

bool status_json_save(struct status_json_s* status) {
    bool ret = true;
    AUTODELETE(status_json) = cJSON_CreateObject();

    do
    {
        /* Prepare JSON object */
        if (cJSON_AddStringToObject(status_json, "updater_version", status->updater_version) == NULL) {
            debug_log("STATUS_JSON: failed to add updater_version field");
            ret = false;
            break;
        }

        if (cJSON_AddStringToObject(status_json, "performed_operation", status->performed_operation) == NULL) {
            debug_log("STATUS_JSON: failed to add performed_operation field");
            ret = false;
            break;
        }

        const char* const result_str = status->operation_result == OPERATION_SUCCESS ? "success" : "failure";
        if (cJSON_AddStringToObject(status_json, "operation_result", result_str) == NULL) {
            debug_log("STATUS_JSON: failed to add operation_result field");
            ret = false;
            break;
        }

        /* Open the file */
        AUTOCLOSE(status_json_fp) = fopen(status->file_path, "w");
        if (status_json_fp == NULL) {
            debug_log("STATUS_JSON: failed to open path: %s", status->file_path);
            ret = false;
            break;
        }

        /* Convert JSON to string */
        AUTOFREE(json_string) = cJSON_Print(status_json);
        if (json_string == NULL) {
            debug_log("STATUS_JSON: failed to convert JSON to string");
            ret = false;
            break;
        }

        /* Write to the file */
        if (fprintf(status_json_fp, "%s", json_string) < 0) {
            debug_log("STATUS_JSON: failed to write to file");
            ret = false;
            break;
        }

    } while (0);

    return ret;
}

const char* status_json_boot_reason_to_operation_str(enum system_boot_reason_code code) {
    switch (code)
    {
        case system_boot_reason_update:
            return "update";
        case system_boot_reason_recovery:
            return "recovery";
        case system_boot_reason_factory:
            return "factory_reset";
        case system_boot_reason_pgm_keys:
            return "program_keys";
        case system_boot_reason_unknown:
            return "unknown";
        default:
            return "not in enum system_boot_reason_code";
    }
}
