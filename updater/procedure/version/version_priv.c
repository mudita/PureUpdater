#include <errno.h>
#include <stdlib.h>

#include "version.h"
#include "version_priv.h"

const char *version_str_fmt = "%d.%d.%d"; // xxx.xxx.xxx\0
#define VERSION_STR_LEN (12)

version_s version_get(const version_json_s *version_json, const char *file_name) {
    version_s version;
    memset(&version, 0, sizeof(version));

    if (version_json == NULL) {
        debug_log("Version: version handle is null");
        goto exit;
    }

    if (version_json->valid && file_name != NULL) {
        if (version_parse_str(&version, json_get_file_from_version(version_json, file_name).version) < 0) {
            goto exit;
        }
    } else {
        debug_log("Version: version is not valid");
        goto exit;
    }

    exit:
    return version;
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

int version_parse_str(version_s *version, const char *version_str) {
    int ret = 0;
    char *version_str_copy = NULL;
    char *token = NULL;
    version_s version_temp;
    version_temp.valid = true;
    const size_t version_str_len = strlen(version_str);

    if (version == NULL || version_str == NULL) {
        debug_log("Version: version handle is null");
        goto fail;
    }

    version_str_copy = strndup(version_str, version_str_len);
    if (version_str_copy == NULL) {
        debug_log("Version: cannot duplicate version string");
        goto fail;
    }

    token = strtok(version_str_copy, ".");
    if (token == NULL) {
        debug_log("Version: parsing error, errno: %d", errno);
        goto fail;
    }
    version_temp.major = strtol(token, NULL, 10);

    token = strtok(NULL, ".");
    if (token == NULL) {
        debug_log("Version: parsing error, errno: %d", errno);
        goto fail;
    }
    version_temp.minor = strtol(token, NULL, 10);

    token = strtok(NULL, ".");
    if (token == NULL) {
        debug_log("Version: parsing error, errno: %d", errno);
        goto fail;
    }
    version_temp.patch = strtol(token, NULL, 10);

    if (version_validate(&version_temp)) {
        version_temp.str = strndup(version_str, version_str_len);
        *version = version_temp;
        goto exit;
    } else {
        debug_log("Version: version is invalid");
        goto fail;
    }

    fail:
    ret = -1;
    version_temp.valid = false;
    exit:
    free(version_str_copy);
    return ret;
}
