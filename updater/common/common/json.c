#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <common/match.h>
#include <common/path_opts.h>

#include "json.h"
#include "json_priv.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

version_json_s json_get_version_struct(trace_t *trace, const char *json_path) {
    cJSON *json = NULL;
    version_json_s version_json;
    version_json.valid = true;

    json = json_get(trace, json_path);
    if (json == NULL) {
        trace_write(trace, JsonInvalidFilePaths, errno);
        goto exit;
    }

    version_json.bootloader = json_get_file_struct(trace, json, "bootloader");
    version_json.boot = json_get_file_struct(trace, json, "boot");
    version_json.updater = json_get_file_struct(trace, json, "updater");

    exit:
    cJSON_Delete(json);
    return version_json;
}

version_json_file_s json_get_file_from_version(trace_t *trace, const version_json_s *version_json, const char *name) {
    version_json_file_s failure_return = {.valid = false};

    if (string_match_end(name, "ecoboot.bin")) {
        return version_json->bootloader;
    } else if (string_match_end(name, "boot.bin")) {
        return version_json->boot;
    } else if (string_match_end(name, "updater.bin")) {
        trace_printf(trace, "Updater");
        return version_json->updater;
    } else {
        trace_write(trace, JsonItemNotFound, errno);
        trace_printf(trace, name);
    }

    return failure_return;
}

version_json_s json_get_fallback()
{
    version_json_s j = {.boot       = {.name = "boot.bin", .md5sum = "", .version = "0.0.0", .valid = true},
                        .bootloader = {.name = "ecoboot.bin", .md5sum = "", .version = "0.0.0", .valid = true},
                        .updater    = {.name = "updater.bin", .md5sum = "", .version = "0.0.0", .valid = true}};
    return j;
}

verify_file_handle_s json_get_verify_files(trace_t *t, const char *new_version, const char *current_version)
{
    verify_file_handle_s verify_handle;
    verify_handle.version_json = json_get_version_struct(t, new_version);
    verify_handle.current_version_json =
        path_check_if_exists(current_version) ? json_get_version_struct(t, current_version) : json_get_fallback();
    return verify_handle;
}

const char *strerror_json(int err) {
    switch (err) {
        case JsonOk:
            return "JsonOk";
        case JsonInvalidFilePaths:
            return "JsonInvalidFilePaths";
        case JsonParseFailed:
            return "JsonParseFailed";
        case JsonReadFailed:
            return "JsonReadFailed";
        case JsonFileTooBig:
            return "JsonFileTooBig";
    }
    return "Unknown";
}

const char *strerror_json_ext(int err, int err_ext) {
    UNUSED(err);
    UNUSED(err_ext);
    return "Unknown";
}
