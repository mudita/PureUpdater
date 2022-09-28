#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <common/tar.h>
#include <common/match.h>
#include <common/path_opts.h>
#include <common/boot_files.h>
#include "dir_walker.h"
#include "priv_backup.h"

static void _autofree(char **f) {
    free(*f);
}

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

#define AUTOFREE(var) char* var __attribute__((__cleanup__(_autofree)))

static const char *user_file_types[] =
{
    ".db",
    ".log"
};

static const char *db_file_types[] =
{
    ".db"
};

static const size_t user_file_types_size = ARRAY_SIZE(user_file_types);
static const size_t db_file_types_size = ARRAY_SIZE(db_file_types);

struct list_node_t {
    struct list_node_t *next;
    char *data;
};

struct get_file_data_t {
    struct list_node_t *root;
    struct list_node_t *node;
    const char **file_types;
    size_t file_types_cnt;
};

static struct list_node_t *list_create() {
    struct list_node_t *node = (struct list_node_t *) calloc(1, sizeof(struct list_node_t));
    return node;
}

/// create new node and point to it
static void list_make_next(struct list_node_t **prev) {
    (*prev)->next = (struct list_node_t *) calloc(1, sizeof(struct list_node_t));
    *prev = (*prev)->next;
}

static void list_free(struct list_node_t **begin) {
    if (NULL == begin) {
        return;
    }
    free((*begin)->data);
    (*begin)->data = NULL;
    if ((*begin)->next) {
        list_free(&((*begin)->next));
    }
    free(*begin);
    *begin = NULL;
}

static int flat_dir_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *data) {
    int ret = 0;
    struct get_file_data_t *get_file_data = (struct get_file_data_t *) (data);

    AUTOFREE(path_to_sanitize) = (char *) calloc(1, strlen(path) + 1);
    AUTOFREE(dir_root) = (char *) calloc(1, strlen(h->root_catalog) + 1);
    if (path_to_sanitize == NULL || dir_root == NULL) {
        debug_log("Backup: failed to allocate memory for path");
        ret = 1;
        goto exit;
    }
    strcpy(path_to_sanitize, path);
    strcpy(dir_root, h->root_catalog);

    char *sanitized = path_sanitize(dir_root, path_to_sanitize);

    switch (what) {
        case DirHandlingDir:
            /// ignore catalogs = deliberately flat
            h->user_break = true;
            break;
        case DirHandlingFile:
            if (string_match_any_of(sanitized, get_file_data->file_types, get_file_data->file_types_cnt)) {
                get_file_data->node->data = calloc(1, strlen(sanitized) + 1);
                if (get_file_data->node->data == NULL) {
                    debug_log("Backup: failed to allocate memory for sanitized path");
                    goto exit;
                }
                strcpy(get_file_data->node->data, sanitized);
                list_make_next(&get_file_data->node);
            }
            break;
        default:
            ret = BackupErrorAny;
            break;
    }

    exit:
    return ret;
}

static int get_files_flat(const char *path, const char **file_types, size_t file_types_cnt, struct list_node_t *nodes) {
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;

    struct get_file_data_t data;
    data.root = nodes;
    data.node = data.root;
    data.file_types = file_types;
    data.file_types_cnt = file_types_cnt;

    recursive_dir_walker_init(&handle_walk, flat_dir_callback, &data);
    recursive_dir_walker(path, &handle_walk, &recursion_limit);
    recursive_dir_walker_deinit(&handle_walk);

    return handle_walk.error;
}

static int tar_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *data) {

    int ret = 0;
    struct tar_ctx *ctx = (struct tar_ctx *) (data);

    AUTOFREE(path_to_sanitize) = (char *) calloc(1, strlen(path) + 1);
    AUTOFREE(dir_root) = (char *) calloc(1, strlen(h->root_catalog) + 1);
    if (path_to_sanitize == NULL || dir_root == NULL) {
        debug_log("Backup: failed to allocate memory for path");
        ret = 1;
        goto exit;
    }
    strcpy(path_to_sanitize, path);
    strcpy(dir_root, h->root_catalog);

    char *sanitized = path_sanitize(dir_root, path_to_sanitize);

    switch (what) {
        case DirHandlingDir:
            ret = tar_catalog(ctx, sanitized);
            break;
        case DirHandlingFile:
            ret = tar_file(ctx, path, sanitized);
            break;
        default:
            ret = BackupErrorAny;
            break;
    }

    exit:
    return ret;
}

static bool backup_files_by_types(const char *src_path, const char *tar_name, const char **file_types, size_t file_types_cnt) {
    bool success = true;
    struct tar_ctx ctx;
    do {
        if (0 != tar_init(&ctx, tar_name, "a")) {
            debug_log("Backup: unable to init tar archive: %s", tar_name);
            success = false;
            break;
        }

        struct list_node_t *nodes = list_create();
        if (nodes == NULL) {
           debug_log("Backup: failed to allocate memory for files list");
           success = false;
           break;
        }

        int ret = get_files_flat(src_path, file_types, file_types_cnt, nodes);
        if (ret != 0) {
            debug_log("Backup: failed to get files list: %d", ret);
            success = false;
            break;
        }

        struct list_node_t *node = nodes;
        while (node != NULL) {
            if (node->data == NULL) {
                node = node->next;
                continue;
            }
            size_t filename_from_length = strlen(node->data) + strlen(src_path) + 2;
            char *filename_from = (char *) calloc(1, filename_from_length);
            if (filename_from == NULL) {
                debug_log("Backup: failed to allocate memory for filename string");
                success = false;
                break;
            }
            snprintf(filename_from, filename_from_length, "%s/%s", src_path, node->data);
            path_remove_dup_slash(filename_from);
            if (0 != tar_file(&ctx, filename_from, node->data)) {
                free(filename_from);
                debug_log("Backup: backing up file %s failed", filename_from);
                success = false;
                break;
            }
            node = node->next;
            free(filename_from);
        }
        list_free(&nodes);

    } while (0);

    if (0 != tar_deinit(&ctx)) {
        debug_log("Backup: tar deinit failed");
        success = false;
    }
    return success;
}

bool backup_boot_files(const struct backup_handle_s *handle) {
    bool success = true;
    struct tar_ctx ctx;
    debug_log("Backup: backing up boot files to %s", handle->backup_to);
    do {
        if (0 != tar_init(&ctx, handle->backup_to, "a")) {
            debug_log("Backup: unable to init tar archive: %s", handle->backup_to);
            success = false;
            break;
        }

        for (size_t i = 0; i < boot_files_to_backup_list_size; ++i) {
            const char *filename = boot_files_to_backup[i];
            size_t filename_from_length = strlen(filename) + strlen(handle->backup_from_os) + 2;
            char *filename_from = (char *) calloc(1, filename_from_length);
            if (filename_from == NULL) {
                debug_log("Backup: failed to allocate memory for filename string");
                success = false;
                break;
            }
            snprintf(filename_from, filename_from_length, "%s/%s", handle->backup_from_os, filename);
            path_remove_dup_slash(filename_from);
            if (0 != tar_file(&ctx, filename_from, filename)) {
                free(filename_from);
                success = false;
                debug_log("Backup: backing up file %s failed", filename);
                break;
            }
            free(filename_from);
        }

    } while (0);

    if (0 != tar_deinit(&ctx)) {
        debug_log("Backup: tar deinit failed");
        success = false;
    }

    return success;
}

bool backup_user_data(const struct backup_handle_s *handle) {
    debug_log("Backup: backing up user data to %s", handle->backup_to);
    return backup_files_by_types(handle->backup_from_user, handle->backup_to, user_file_types, user_file_types_size);
}

bool backup_databases(const struct backup_handle_s *handle) {
    debug_log("Backup: backing up databases to %s", handle->backup_to);
    return backup_files_by_types(handle->backup_from_user, handle->backup_to, db_file_types, db_file_types_size);
}

bool check_backup_entries(const struct backup_handle_s *handle) {
    debug_log("Backup: checking backup paths");
    bool ret = handle->backup_from_os != NULL && handle->backup_from_user != NULL && handle->backup_to != NULL;
    if (ret) {
        ret = (strlen(handle->backup_from_os) > 0) && (strlen(handle->backup_from_user) > 0) &&
              (strlen(handle->backup_to) > 0);
    }
    if (!ret) {
        debug_log("Backup: checking backup paths failed: %d", ret);
    }
    return ret;
}

bool backup_whole_directory(const struct backup_handle_s *handle) {
    bool success = true;

    debug_log("Backup: backing up whole directory");
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;
    struct tar_ctx ctx;

    do {
        if (0 != tar_init(&ctx, handle->backup_to, "w")) {
            debug_log("Backup: unable to init tar archive: %s", handle->backup_to);
            success = false;
            break;
        }

        recursive_dir_walker_init(&handle_walk, tar_callback, &ctx);
        recursive_dir_walker(handle->backup_from_os, &handle_walk, &recursion_limit);
        recursive_dir_walker(handle->backup_from_user, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            debug_log("Backup: walker error: %d", handle_walk.error);
            success = false;
            break;
        }

    } while (0);

    if (tar_deinit(&ctx) != 0) {
        debug_log("Backup: tar deinit failed");
        success = false;
    }
    return success;
}
