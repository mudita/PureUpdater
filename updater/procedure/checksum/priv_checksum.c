#include "checksum.h"
#include "priv_checksum.h"
#include <errno.h>
#include "string.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <common/json.h>

const char *checksum_label = "checksums";

const char *get_checksum(trace_list_t *tl, const cJSON *json, const char *file_name) {
    const cJSON *checksums_tab = NULL;
    const cJSON *file_checksum = NULL;

    if (tl == NULL) {
        goto exit;
    }

    trace_t *trace = trace_append("get_checksum", tl, strerror_checksum, NULL);

    if (json != NULL && file_name != NULL) {
        checksums_tab = get_from_json(tl, json, checksum_label);
        if ( checksums_tab == NULL ) {
            goto exit;
        }
        file_checksum = get_from_json(tl, checksums_tab, file_name);
        if (file_checksum != NULL) {
            return file_checksum->valuestring;
        } else {
            trace_write(trace, ChecksumNotFoundInJson, errno);
            trace_printf(trace, file_name);
            goto exit;
        }
    }

    exit:
    return NULL;
}

bool compare_checksums(const char *checksum_l, const char *checksum_r) {
    return (strcmp(checksum_l, checksum_r) == 0);
}
