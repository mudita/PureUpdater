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

cJSON *json_get(trace_list_t *tl, const char *json_path) {
    cJSON *ret = NULL;
    const uint16_t json_buffer_size = 1024;
    char buffer[json_buffer_size];

    off_t file_size = 0;
    int bytes_read = 0;

    if (tl == NULL) {
        printf("json_get trace list null");
        goto exit;
    }

    trace_t *trace = trace_append("json_get", tl, strerror_json, NULL);

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

cJSON *json_get_item_from(trace_list_t *tl, const cJSON *json, const char *name) {
    cJSON *object = NULL;

    if (tl == NULL) {
        printf("json_get_item_from trace list null");
        goto exit;
    }

    trace_t *trace = trace_append("json_get_item_from", tl, strerror_json, NULL);

    if (json != NULL && name != NULL) {
        object = cJSON_GetObjectItemCaseSensitive(json, name);
    } else {
        trace_write(trace, JsonItemNotFound, errno);
    }

    exit:
    return object;
}

version_json_file_s json_get_file_struct(trace_list_t *tl, const cJSON *json, const char *filename_arg) {
    version_json_file_s file_version;
    file_version.valid = true;
    cJSON *name = NULL;
    cJSON *filename = NULL;
    cJSON *checksum = NULL;
    cJSON *version = NULL;

    if (tl == NULL) {
        printf("json_get_file_struct trace list null");
        file_version.valid = false;
        goto exit;
    }

    trace_t *trace = trace_append("json_get_file_struct", tl, strerror_json, NULL);

    name = json_get_item_from(tl, json, filename_arg);
    if (name == NULL) {
        goto fail;
    }

    filename = json_get_item_from(tl, name, "filename");
    if (cJSON_IsString(filename) && filename->valuestring != NULL) {
        strcpy(file_version.name, filename->valuestring);
    } else {
        goto fail;
    }

    checksum = json_get_item_from(tl, name, "md5sum");
    if (cJSON_IsString(checksum) && checksum->valuestring != NULL) {
        strcpy(file_version.md5sum, checksum->valuestring);
    } else {
        goto fail;
    }

    version = json_get_item_from(tl, name, "version");
    if (cJSON_IsString(version) && version->valuestring != NULL) {
        strcpy(file_version.version, version->valuestring);
    } else {
        goto fail;
    }

    goto exit;

    fail:
    trace_write(trace, JsonItemNotFound, errno);
    strcpy(file_version.name, filename_arg);
    strcpy(file_version.md5sum, "NULL");
    strcpy(file_version.version, "NULL");
    file_version.valid = false;
    exit:
    return file_version;
}
