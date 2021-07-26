#include <errno.h>
#include <stdlib.h>

#include "version.h"
#include "version_priv.h"

version_s version_get(trace_t *trace, const version_json_s *version_json, const char *file_name) {
    version_s version;

    if (trace == NULL) {
        printf("get_version trace/handle null error");
        goto exit;
    }

    if (version_json->valid && file_name != NULL) {
        if (version_parse_str(trace, &version, json_get_file_from_version(trace, version_json, file_name).version) < 0) {
            trace_write(trace, VersionInvalidStringParse, errno);
            goto exit;
        }
    } else {
        trace_write(trace, VersionNotFound, errno);
        goto exit;
    }

    exit:
    return version;
}

const char *version_get_str(version_s *version) {
    sprintf(version->str, "%d.%d.%d", version->major, version->minor, version->patch);
    return version->str;
}

bool version_validate(const version_s *version) {
    return (version->major >= 0 && version->major <= 99) &&
           (version->minor >= 0 && version->minor <= 99) &&
           (version->patch >= 0 && version->patch <= 99);
}

bool version_is_lhs_newer(const version_s *version_l, const version_s *version_r) {
    bool ret = true;

    if (version_l->major < version_r->major) {
        ret = false;
    } else if (version_l->major > version_r->major) {
        ret = true;
    } else {
        if (version_l->minor < version_r->minor) {
            ret = false;
        } else if (version_l->minor > version_r->minor) {
            ret = true;
        } else {
            if (version_l->patch < version_r->patch) {
                ret = false;
            } else if (version_l->patch >= version_r->patch) {
                ret = true;
            }
        }
    }

    return ret;
}

int version_parse_str(trace_t *trace, version_s *version, const char *version_str) {
    int ret = 0;
    char *version_str_copy = NULL;
    char *token = NULL;
    version_s version_temp;
    version_temp.valid = true;

    if (trace == NULL) {
        printf("parse_version_str trace null error");
        goto fail;
    }

    if (version == NULL || version_str == NULL) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto fail;
    }

    version_str_copy = (char *) malloc(strlen(version_str) + 1);
    if (version_str_copy == NULL) {
        trace_write(trace, VersionAllocError, errno);
        goto fail;
    }

    strcpy(version_str_copy, version_str);

    token = strtok(version_str_copy, ".");
    if (token == NULL) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto fail;
    }
    version_temp.major = strtol(token, NULL, 10);

    token = strtok(NULL, ".");
    if (token == NULL) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto fail;
    }
    version_temp.minor = strtol(token, NULL, 10);

    token = strtok(NULL, ".");
    if (token == NULL) {
        trace_write(trace, VersionInvalidStringParse, errno);
        goto fail;
    }
    version_temp.patch = strtol(token, NULL, 10);

    if (version_validate(&version_temp)) {
        strcpy(version->str, version_str);
        *version = version_temp;
        goto exit;
    } else {
        trace_write(trace, VersionInvalidNumber, errno);
        goto fail;
    }

    fail:
    ret = -1;
    version_temp.valid = false;
    exit:
    free(version_str_copy);
    return ret;
}
