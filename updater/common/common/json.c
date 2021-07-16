#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>

#include "json.h"

static void _autoclose(int *f) {
    if (*f > 0) {
        close(*f);
    }
}

#define UNUSED(expr) do { (void)(expr); } while (0)
#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))

cJSON *get_json(trace_list_t *tl, const char *json_path) {
    cJSON *ret = NULL;
    const uint16_t json_buffer_size = 1024;
    char buffer[json_buffer_size];

    off_t file_size = 0;
    int bytes_read = 0;

    if (tl == NULL) {
        printf("get_json trace list null");
        goto exit;
    }

    trace_t *trace = trace_append("get_json", tl, strerror_json, NULL);

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

cJSON *get_from_json(trace_list_t *tl, const cJSON * json, const char *name) {
    cJSON *object = NULL;

    if (tl == NULL) {
        printf("get_from_json trace list null");
        goto exit;
    }

    trace_t *trace = trace_append("get_from_json", tl, strerror_json, NULL);

    if (json != NULL && name != NULL) {
        object = cJSON_GetObjectItemCaseSensitive(json, name);
    }else{
        trace_write(trace, JsonItemNotFound, errno);
    }

    exit:
    return object;
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
