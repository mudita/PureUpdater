#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <common/trace.h>
#include <common/path_opts.h>
#include <common/enum_s.h>
#include <common/boot_files.h>
#include <procedure/backup/dir_walker.h>
#include "priv_tmp.h"

struct unlink_data_s {
    trace_t *t;
    bool factory_reset;
};

enum tmp_error_e {
    ErrorTmpOk = 0,
    ErrorTmpFs,
    ErrorTmpWalk,
};

static const char *strerror_err(int err) {
    switch (err) {
        ENUMS(ErrorTmpWalk);
        ENUMS(ErrorTmpFs);
        ENUMS(ErrorTmpOk);
    }
    return "";
}

static const char *strerror_ext(int err, int ext_err) {
    if (err && ext_err)
        return strerror(ext_err);
    return "";
}

int unlink_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;
    struct unlink_data_s *data = (struct unlink_data_s *) (d);

    switch (what) {
        case DirHandlingDir:
            break;
        case DirHandlingFile:
            if (data->factory_reset) {
                for(size_t i = 0; i < db_extensions_list_size; ++i){
                    const char *extension = strrchr(path, '.');
                    if(extension == NULL){
                      printf("rem file without extension: %s %d\n", path, ret);
                      ret = unlink(path);
                      break;
                    } else if (strcmp(db_extensions[i], extension) == 0) {
                        printf("rem file: %s %d\n", path, ret);
                        ret = unlink(path);
                        break;
                    }
                }
            }else{
                printf("rem file: %s %d\n", path, ret);
                ret = unlink(path);
            }
            break;
        default:
            ret = 1;
            break;
    }
    if (ret != 0) {
        trace_write(data->t, ErrorTmpFs, ret);
    }
    return ret;
}

static int remove_dir_callback(const char *path, struct dir_handler_s *h, void *d) {
    (void) h;
    struct unlink_data_s *data = (struct unlink_data_s *) (d);
    int ret = 0;

    if(data->factory_reset){
        goto exit;
    }

    ret = rmdir(path);
    if (ret != 0) {
        trace_write(data->t, ErrorTmpFs, ret);
    }
    printf("rem catalog: %s %d\n", path, ret);

    exit:
    return ret;
}

bool recursive_unlink(const char *what, bool factory_reset, trace_t *t) {
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        struct unlink_data_s data = {t, factory_reset};
        recursive_dir_walker_init(&handle_walk, unlink_callback, &data);
        handle_walk.callback_dir_closed = remove_dir_callback;
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            trace_write(t, ErrorTmpWalk, errno);
            success = false;
            break;
        }
    } while (0);

    return success;
}

static bool create_single(const char *what, struct update_handle_s *handle, trace_t *t) {
    (void) handle;
    bool success = true;

    struct stat data;
    int ret = stat(what, &data);
    if (ret == 0 && !recursive_unlink(what, NULL, t)) {
        trace_write(t, ErrorTmpWalk, errno);
        success = false;
        goto exit;
    }
    if (ret == 0 && rmdir(what) != 0) {
        trace_write(t, ErrorTmpFs, errno);
        trace_printf(t, what);
        success = false;
        goto exit;
    }
    if (ret != 0 && errno == ENOENT) {
        ret = 0;
    }

    if (mkdir(what, 0666) != 0) {
        trace_write(t, ErrorTmpFs, errno);
        trace_printf(t, what);
        success = false;
        goto exit;
    }

    exit:
    return success;
}

bool tmp_create_catalog(struct update_handle_s *handle, trace_list_t *tl) {
    bool retval = true;
    trace_t *t = trace_append("mktemp", tl, strerror_err, strerror_ext);
    printf("create temp catalog\n");

    if (!create_single(handle->tmp_os, handle, t)) {
        trace_write(t, ErrorTmpWalk, 0);
        retval = false;
        goto exit;
    }

    if (!create_single(handle->tmp_user, handle, t)) {
        trace_write(t, ErrorTmpWalk, ErrorTmpWalk);
        retval = false;
        goto exit;
    }

    exit:
    return retval;
}


struct mv_data_s {
    trace_t *t;
    const char *to;
};

int mv_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *d) {

    int ret = 0;
    (void) h;
    struct mv_data_s *data = (struct mv_data_s *) (d);

    char *path_to_sanitize = strndup(path, strlen(path));
    char *dir_root = strndup(h->root_catalog, strlen(h->root_catalog));
    char *sanitized = path_sanitize(dir_root, path_to_sanitize);
    char *final_path = (char *) calloc(1, strlen(sanitized) + strlen(data->to) + 2);
    sprintf(final_path, "%s/%s", data->to, sanitized);

    printf("final_path: %s\n", final_path);

    switch (what) {
        case DirHandlingDir:
            ret = mkdir(final_path, 0666);
            if (ret != 0 && errno == EEXIST) {
                printf("mkdir: %s %d\n", final_path, ret);
                ret = 0;
            }
            printf("mkdir: %s %d\n", final_path, ret);
            break;
        case DirHandlingFile: {
            struct stat data;
            ret = stat(final_path, &data);
            if (ret == 0) {
                ret = unlink(final_path);
                if (ret != 0) {
                    printf("unlink old file failed");
                    goto exit;
                }
            }
            ret = rename(path, final_path);
            if (ret)
                printf("rename %s -> %s: %d %d %s\n", path, final_path, ret, errno, strerror(errno));
        }
            break;
        default:
            break;
    }
    exit:
    if (ret != 0) {
        trace_write(data->t, ErrorTmpFs, errno);
    }
    free(final_path);
    free(dir_root);
    free(path_to_sanitize);
    return ret;
}

static bool recursive_mv(const char *what, const char *where, trace_t *t) {
    (void) where;
    bool success = true;
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    do {
        printf("move data to: %s\n", where);
        struct mv_data_s data = {t, NULL};
        data.to = where;
        recursive_dir_walker_init(&handle_walk, mv_callback, &data);
        recursive_dir_walker(what, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            trace_write(t, ErrorTmpWalk, errno);
            trace_printf(t, what);
            success = false;
            break;
        }

        printf("remove data after move %s\n", what);
        if (!recursive_unlink(what, NULL, t) != 0) {
            trace_write(t, ErrorTmpWalk, errno);
            trace_printf(t, what);
            success = false;
            break;
        }

    } while (0);

    return success;
}

bool tmp_files_move(struct update_handle_s *handle, trace_list_t *tl) {
    bool success = true;

    trace_t *t = trace_append("move", tl, strerror_err, strerror_ext);

    printf("move user data...\n");
    if (!recursive_mv(handle->tmp_os, handle->update_os, t)) {
        trace_write(t, ErrorTmpWalk, errno);
        success = false;
        goto exit;
    }

    if (!recursive_mv(handle->tmp_user, handle->update_user, t)) {
        trace_write(t, ErrorTmpWalk, errno);
        success = false;
        goto exit;
    }

    exit:
    return success;
}
