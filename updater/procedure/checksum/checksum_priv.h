#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

bool checksum_compare(const char *checksum_l, const char *checksum_r);

void checksum_get_readable(const unsigned char *hex, char * readable);

#ifdef __cplusplus
}
#endif
