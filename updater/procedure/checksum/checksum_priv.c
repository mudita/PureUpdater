#include <stdio.h>
#include <string.h>

#include "checksum_priv.h"

bool checksum_compare(const char *checksum_l, const char *checksum_r) {
    return (strcmp(checksum_l, checksum_r) == 0);
}

void checksum_get_readable(const unsigned char *hex, char *readable) {
    char *ptr = &readable[0];
    for (size_t i = 0; i < 16; ++i) {
        ptr += sprintf(ptr, "%02x", hex[i]);
    }
}
