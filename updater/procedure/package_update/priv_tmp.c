#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <common/path_opts.h>
#include <common/enum_s.h>
#include <common/boot_files.h>
#include <procedure/backup/dir_walker.h>
#include "priv_tmp.h"

static void _autofree(char **f) {
    free(*f);
}

#define AUTOFREE(var) char* var __attribute__((__cleanup__(_autofree)))
#define UNUSED(expr) do { (void)(expr); } while (0)
#define PATH_BUF_SIZE 128

struct unlink_data_s {
    bool factory_reset;
};

enum tmp_error_e {
    ErrorTmpOk = 0,
    ErrorTmpFs,
    ErrorTmpWalk,
};

int unlink_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;
    struct unlink_data_s *data = (struct unlink_data_s *) (d);

    switch (what) {
        case DirHandlingDir:
            break;
        case DirHandlingFile:
            if (data->factory_reset) {
                for (size_t i = 0; i < db_extensions_list_size; ++i) {
                    const char *extension = strrchr(path, '.');
                    if (extension == NULL) {
                        ret = unlink(path);
                        debug_log("Unlink: removed file without extension: %s", path);
                        break;
                    } else if (strcmp(db_extensions[i], extension) == 0) {
                        debug_log("Unlink: removing file: %s", path);
                        ret = unlink(path);
                        break;
                    }
                }
            } else {
                debug_log("Unlink: removing file: %s", path);
                ret = unlink(path);
            }
            break;
        default:
            ret = 1;
            break;
    }
    if (ret != 0) {
        debug_log("Unlink: unlink error, errno %d", errno);
    }
    return ret;
}

static int remove_dir_callback(const char *path, struct dir_handler_s *h, void *d) {
    UNUSED(h);
    struct unlink_data_s *data = (struct unlink_data_s *) (d);
    int ret = 0;

    if (data->factory_reset) {
        goto exit;
    }

    ret = rmdir(path);
    if (ret != 0) {
        debug_log("Rmdir: rmdir error, errno %d", errno);
    }
    debug_log("Rmdir: removed catalog: %s", path);

    exit:
    return ret;
}

bool recursive_unlink(const char *what, bool factory_reset) {
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct unlink_data_s data = {factory_reset};
        recursive_dir_walker_init(&handle_walk, unlink_callback, &data);
        handle_walk.callback_dir_closed = remove_dir_callback;
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Unlink: walker error, errno: %d", errno);
            success = false;
            break;
        }
    } while (0);

    return success;
}

static bool create_single(const char *what, struct update_handle_s *handle) {
    (void) handle;
    bool success = true;

    struct stat data;
    int ret = stat(what, &data);
    if (ret == 0 && !recursive_unlink(what, NULL)) {
        success = false;
        goto exit;
    }
    if (ret == 0 && rmdir(what) != 0) {
        debug_log("Create dir: unable to remove directory: %s", what);
        success = false;
        goto exit;
    }
    if (ret != 0 && errno == ENOENT) {
        ret = 0;
    }

    if (mkdir(what, 0666) != 0) {
        debug_log("Create dir: failed to create a directory %s, errno %d", what, errno);
        success = false;
        goto exit;
    }

    exit:
    return success;
}

bool tmp_create_catalog(struct update_handle_s *handle) {
    bool retval = true;
    debug_log("Tmp: create temp catalog");

    if (!create_single(handle->tmp_os, handle)) {
        retval = false;
        goto exit;
    }

    if (!create_single(handle->tmp_user, handle)) {
        retval = false;
        goto exit;
    }

    exit:
    return retval;
}


struct mv_data_s {
    const char *to;
};

int mv_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {
    int ret = 0;
    (void) h;
    struct mv_data_s *data = (struct mv_data_s *) (d);

    AUTOFREE(path_to_sanitize) = (char *) calloc(1, strlen(path) + 1);
    AUTOFREE(dir_root) = (char *) calloc(1, strlen(h->root_catalog) + 1);
    if (path_to_sanitize == NULL || dir_root == NULL) {
        debug_log("Move: failed to allocate memory for paths");
        ret = 1;
        goto exit;
    }

    strcpy(path_to_sanitize, path);
    strcpy(dir_root, h->root_catalog);
    char *sanitized_path = path_sanitize(dir_root, path_to_sanitize);

    size_t path_length = strlen(sanitized_path) + strlen(data->to) + 2;

    AUTOFREE(final_path) = (char *) calloc(1, path_length);
    if (final_path == NULL) {
        debug_log("Move: failed to allocate memory for final path");
        ret = 1;
        goto exit;
    }
    snprintf(final_path, path_length, "%s/%s", data->to, sanitized_path);

    debug_log("Move: path: %s", final_path);

    switch (what) {
        case DirHandlingDir:
            ret = mkdir(final_path, 0666);
            if (ret != 0 && errno == EEXIST) {
                debug_log("Move: mkdir - directory already exists %s", final_path);
                ret = 0;
            }
            debug_log("Move: created directory %s", final_path);
            break;
        case DirHandlingFile: {
            struct stat data;
            ret = stat(final_path, &data);
            if (ret == 0) {
                ret = unlink(final_path);
                if (ret != 0) {
                    debug_log("Move: unlinking old file failed, errno %d", errno);
                    goto exit;
                }
            }
            ret = rename(path, final_path);
            if (ret) {
                debug_log("Move: rename %s -> %s failed, errno %d (%s)\n", path, final_path, errno, strerror(errno));
            }
        }
            break;
        default:
            break;
    }
    exit:
    return ret;
}

static bool recursive_mv(const char *what, const char *where) {
    (void) where;
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct mv_data_s data = {NULL};
        data.to = where;
        recursive_dir_walker_init(&handle_walk, mv_callback, &data);
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Move: walker error, errno: %d", errno);
            success = false;
            break;
        }

        debug_log("Move: removing data after moving: %s", what);
        if (!recursive_unlink(what, NULL) != 0) {
            success = false;
            break;
        }

    } while (0);

    return success;
}

bool tmp_files_move(struct update_handle_s *handle) {
    bool success = true;
    char path_buf[PATH_BUF_SIZE];

    debug_log("Move: move OS data from %s to %s...", handle->tmp_os, handle->update_os);
    do
    {
        if (!recursive_mv(handle->tmp_os, handle->update_os)) {
            success = false;
            break;
        }

        /* In both update and recovery archive the files to be placed
         * on "/user" partition are inside folder "user".
         * Move the contents of the folder, not the folder itself. */
        snprintf(path_buf, PATH_BUF_SIZE, "%s/user", handle->tmp_user);

        debug_log("Move: move user data from %s to %s...", path_buf, handle->update_user);
        if (!recursive_mv(path_buf, handle->update_user)) {
            success = false;
            break;
        }

    } while (0);

    snprintf(path_buf, PATH_BUF_SIZE, "%s/.directory_is_indexed", handle->update_user);
    debug_log("Move: remove %s to force user partition reindexing", path_buf);
    if (unlink(path_buf) != 0) {
        debug_log("Move: failed to remove %s, errno: %d", path_buf, errno);
    }

    return success;
}
