#include "path_opts.h"
#include <string.h>

void path_remove_dup_slash(char *from)
{
    for (size_t i = 0; i < strlen(from) - 1; ++i) {
        if (from[i] == '/' && from[i + 1] == '/') {
            memcpy(from + i, from + i + 1, strlen(from + i + 1) + 1);
        }
    }
}

void path_remove_cwd(char *from)
{
    if ( strlen(from) >= 2 && strncmp(from, "./", 2) == 0) 
    {
        memcpy(from, from + 2, strlen(from) + 1);
    }
}

/// remove dup from path and entry /
char *path_sanitize(char *from, char *path)
{
    if (strlen(from) > strlen(path)) {
        return 0;
    }
    path_remove_dup_slash(from);
    path_remove_dup_slash(path);
    char *ret = path + strlen(from);
    if (ret[0] == '/') {
        return ret + 1;
    }
    return ret;
}


