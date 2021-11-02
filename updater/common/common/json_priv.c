#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>

#include "json_priv.h"

static void _autoclose(int *f) {
    if (*f > 0) {
        close(*f);
    }
}

#define UNUSED(expr) do { (void)(expr); } while (0)
#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))

cJSON *json_get(trace_t *trace, const char *json_path) {
    cJSON *ret = NULL;
    const uint16_t json_buffer_size = 1024;
    char buffer[json_buffer_size];

    off_t file_size = 0;
    int bytes_read = 0;

    AUTOCLOSE(json_fd) = open(json_path, O_RDONLY);
    if (json_fd <= 0) {
        trace_write(trace, JsonInvalidFilePaths, errno);
        goto exit;
    }

    file_size = lseek(json_fd, 0, SEEK_END);
    lseek(json_fd, 0, SEEK_SET);
    if (file_size > json_buffer_size) {
        trace_write(trace, JsonFileTooBig, errno);
        goto exit;
    }

    memset(buffer, 0, json_buffer_size);
    bytes_read = read(json_fd, buffer, json_buffer_size);
    if (bytes_read < 1) {
        trace_write(trace, JsonReadFailed, errno);
        goto exit;
    }

    ret = cJSON_Parse(buffer);

    if (ret == NULL) {
        const char *err = cJSON_GetErrorPtr();
        if (err != NULL) {
            trace_write(trace, JsonParseFailed, errno);
            trace_printf(trace, err);
        }
    }

    exit:
    return ret;
}

cJSON *json_get_item_from(trace_t *trace, const cJSON *json, const char *name) {
    cJSON *object = NULL;

    if (json != NULL && name != NULL) {
        object = cJSON_GetObjectItemCaseSensitive(json, name);
    } else {
        trace_write(trace, JsonItemNotFound, errno);
    }

    return object;
}

version_json_file_s json_get_file_struct(trace_t *trace, const cJSON *json, const char *filename_arg) {
    version_json_file_s file_version;
    file_version.valid = true;
    cJSON *name = NULL;
    cJSON *filename = NULL;
    cJSON *checksum = NULL;
    cJSON *version = NULL;

    name = json_get_item_from(trace, json, filename_arg);
    if (name == NULL) {
        goto exit;
    }

    filename = json_get_item_from(trace, name, "filename");
    if (cJSON_IsString(filename) && filename->valuestring != NULL) {
        file_version.name = strndup(filename->valuestring, strlen(filename->valuestring));
    } else {
        goto fail;
    }

    checksum = json_get_item_from(trace, name, "md5sum");
    if (cJSON_IsString(checksum) && checksum->valuestring != NULL) {
        file_version.md5sum = strndup(checksum->valuestring, strlen(checksum->valuestring));
    } else {
        goto fail;
    }

    version = json_get_item_from(trace, name, "version");
    if (cJSON_IsString(version) && version->valuestring != NULL) {
        file_version.version = strndup(version->valuestring, strlen(version->valuestring));
    } else {
        goto fail;
    }

    goto exit;

    fail:
    trace_write(trace, JsonItemNotFound, errno);
    file_version.name = strndup(filename_arg, strlen(filename_arg));
    file_version.md5sum = strdup("NULL");
    file_version.version = strdup("NULL");
    file_version.valid = false;
    exit:
    return file_version;
}
