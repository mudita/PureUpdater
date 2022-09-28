#include <string.h>
#include "match.h"

/// compare from back if file_type matches
bool string_match_end(const char *in, const char *file_type) {
    size_t len_file_type = strlen(file_type);
    size_t len_in = strlen(in);
    if (len_in == 0 || len_file_type == 0 || len_in < len_file_type) {
        return false;
    }
    if (strcmp(in + len_in - len_file_type, file_type) != 0) {
        return false;
    }
    return true;
}

bool string_match_partial(const char *in, const char *to_match) {
    size_t len_to_match = strlen(to_match);
    size_t len_in = strlen(in);
    if (len_in == 0 || len_to_match == 0 || len_in < len_to_match) {
        return false;
    }
    if (strstr(in, to_match) == NULL) {
        return false;
    }
    return true;
}

bool string_match_any_of(const char *str, const char **file_table, size_t cnt) {
    for (size_t i = 0; i < cnt; ++i) {
        if (string_match_end(str, file_table[i])) {
            return true;
        }
    }
    return false;
}

bool string_match_any_of_partial(const char *str, const char **file_table, size_t cnt) {
    for (size_t i = 0; i < cnt; ++i) {
        if (string_match_partial(str, file_table[i])) {
            return true;
        }
    }
    return false;
}
