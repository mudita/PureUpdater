#include <errno.h>
#include <memory.h>
#include <common/match.h>
#include <common/path_opts.h>
#include "json.h"
#include "json_priv.h"

#define UNUSED(expr) do { (void)(expr); } while (0)

version_json_s json_get_version_struct(const char *json_path) {
    cJSON *json = NULL;
    version_json_s version_json;
    version_json.valid = true;

    json = json_get(json_path);
    if (json == NULL) {
        goto exit;
    }

    version_json.bootloader = json_get_file_struct(json, "bootloader");
    version_json.boot = json_get_file_struct(json, "boot");
    version_json.updater = json_get_file_struct(json, "updater");

    exit:
    cJSON_Delete(json);
    return version_json;
}

version_json_file_s json_get_file_from_version(const version_json_s *version_json, const char *name) {
    version_json_file_s failure_return = {.valid = false};

    if (string_match_end(name, "ecoboot.bin")) {
        return version_json->bootloader;
    } else if (string_match_end(name, "boot.bin")) {
        return version_json->boot;
    } else if (string_match_end(name, "updater.bin")) {
        return version_json->updater;
    } else {
        debug_log("JSON: failed to get file from version.json");
    }

    return failure_return;
}

version_json_s json_get_fallback() {
    version_json_s j = {.boot       = {.name = "boot.bin", .md5sum = "", .version = "0.0.0", .valid = true},
            .bootloader = {.name = "ecoboot.bin", .md5sum = "", .version = "0.0.0", .valid = true},
            .updater    = {.name = "updater.bin", .md5sum = "", .version = "0.0.0", .valid = true}};
    return j;
}

verify_file_handle_s json_get_verify_files(const char *new_version, const char *current_version) {
    verify_file_handle_s verify_handle;
    verify_handle.version_json = json_get_version_struct(new_version);
    verify_handle.current_version_json =
            path_check_if_exists(current_version) ? json_get_version_struct(current_version) : json_get_fallback();
    return verify_handle;
}