#pragma once

#ifdef __cplusplus
extern "C"
{
#endif


/// remove duplicated slashes from path
void path_remove_dup_slash(char *from);
/// remove ./
void path_remove_cwd(char *from);
/// remove dup from path and entry /
char *path_sanitize(char *from, char *path);
void path_remove_trailing_slash(char* out);

#ifdef __cplusplus
}
#endif
