#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <common/tar.h>
#include <common/match.h>
#include <common/path_opts.h>
#include "dir_walker.h"
#include "priv_backup.h"

const char *boot_files_to_backup[] = 
{
    "boot.bin",
    "version.json",
};

const char *user_file_types_to_backup[] =
{
    ".db"
};

bool backup_boot_partition(struct backup_handle_s *handle, trace_list_t *tl)
{
    bool success = true;
    struct tar_ctx ctx;
    trace_t *t = trace_append("backup_boot", tl, NULL,NULL);
    trace_t *tar_t = trace_append("tar", tl, NULL,NULL);
    do {
        if (0 != tar_init(&ctx,tar_t, handle->backup_to, "a")) {
            trace_write(t,BackupErrorTar,0);
            success = false;
            break;
        }

        for (int i = 0; i < sizeof(boot_files_to_backup) / sizeof(boot_files_to_backup[0]); ++i) {
            const char *filename = boot_files_to_backup[i];
            char *filename_from  = (char *)calloc(1,strlen(filename) + strlen(handle->backup_from) + 2);
            char *filename_to    = (char *)calloc(1,strlen(filename) + strlen(handle->backup_to) + 2);
            sprintf(filename_from, "%s/%s", handle->backup_from, filename);
            sprintf(filename_to, "%s/%s", handle->backup_to, filename);
            path_remove_dup_slash(filename_from);
            path_remove_dup_slash(filename_to);
            if (0 != tar_file(&ctx, filename_from, filename)) {
                free(filename_from);
                success = false;
                break;
            }
            free(filename_from);
            free(filename_to);
        }

        if (0 != tar_deinit(&ctx) || success == false) {
            trace_write(t,BackupErrorTar,0);
            success = false;
            break;
        }
        return success;
    } while (0);
    tar_deinit(&ctx);
    return success;
}

struct list_node_t {
    struct list_node_t *next;
    char *data;
};

struct list_node_t *list_create()
{
    struct list_node_t *node = (struct list_node_t*)calloc(1,sizeof(struct list_node_t));
    return node;
}

/// create new node and point to it
void list_make_next(struct list_node_t **prev)
{
    (*prev)->next = (struct list_node_t*)calloc(1,sizeof(struct list_node_t));
    *prev = (*prev)->next;
}

void list_free(struct list_node_t **begin)
{
    if ( NULL == begin ) {
        return;
    }
    free((*begin)->data);
    (*begin)->data = NULL;
    if ((*begin)->next)
        list_free(&((*begin)->next));
    free(*begin);
    *begin = NULL;
}

struct get_file_data_t
{
    struct list_node_t *root;
    struct list_node_t *node;
    const char** file_types;
    size_t file_types_cnt;
};


int flat_dir_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *data)
{

    int ret             = 0;
    struct get_file_data_t * get_file_data = (struct get_file_data_t *)(data);

    char *pa            = (char *)calloc(1,strlen(path) + 1);
    char *fr            = (char *)calloc(1,strlen(h->root_catalog) + 1);

    strcpy(pa, path);
    strcpy(fr, h->root_catalog);

    char *sanitized = path_sanitize(fr, pa);

    switch (what) {
    case DirHandlingDir:
        /// ignore catalogs = deliberately flat
        h->user_break = true;
        break;
    case DirHandlingFile:
        if (string_match_any_of(sanitized, get_file_data->file_types, get_file_data->file_types_cnt)) {
            get_file_data->node->data = calloc(1,strlen(sanitized) + 1);
            strcpy(get_file_data->node->data, sanitized);
            list_make_next(&get_file_data->node);
        }
        break;
    default:
        ret = BackupErrorAny;
        break;
    }

    free(fr);
    free(pa);
    return ret;
}

int get_files_flat(const char *path, const char **file_types, size_t file_types_cnt, struct list_node_t *nodes)
{
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

bool backup_user_data(struct backup_handle_s *handle, trace_list_t *tl)
{
    bool success = true;
    struct tar_ctx ctx;
    trace_t *t = trace_append("backup_user", tl, NULL,NULL);
    trace_t *tar_t = trace_append("tar", tl, NULL,NULL);
    do {
        if (0 != tar_init(&ctx,tar_t, handle->backup_to, "a")) {
            trace_write(t,BackupErrorTar,0);
            success = false;
            break;
        }

        size_t file_types_cnt     = sizeof(user_file_types_to_backup) / sizeof(user_file_types_to_backup[0]);
        struct list_node_t *nodes = list_create();
        int ret = get_files_flat(handle->backup_from, user_file_types_to_backup, file_types_cnt, nodes);
        if (ret != 0 ) {
            trace_write(t,BackupErrorTar,0);
            tar_deinit(&ctx);
            return false;
        }

        struct list_node_t *node = nodes;
        while (node != NULL) {
            if (node->data == NULL) {
                node = node->next;
                continue;
            }
            char *filename_from = (char *)calloc(1,strlen(node->data) + strlen(handle->backup_from) + 2);
            sprintf(filename_from, "%s/%s", handle->backup_from, node->data);
            path_remove_dup_slash(filename_from);
            if (0 != tar_file(&ctx,filename_from, node->data)) {
                free(filename_from);
                success = false;
                break;
            }
            node = node->next;
            free(filename_from);
        }
        list_free(&nodes);

        if (0 != tar_deinit(&ctx) || success == false) {
            trace_write(t,BackupErrorTar,0);
            success = false;
            break;
        }
        return success;
    } while (0);
    tar_deinit(&ctx);
    return success;
}

bool check_backup_entries(struct backup_handle_s *handle,trace_list_t *tl)
{
    trace_t *t = trace_append("check_backup", tl, NULL,NULL);
    bool ret = handle->backup_from != NULL && handle->backup_to != NULL;
    if (ret) {
        ret = (strlen(handle->backup_from) > 0) && (strlen(handle->backup_to) > 0);
    }
    if (!ret) {
        trace_write(t,BackupBadInput,0);
    }
    return ret;
}

int tar_callback(const char *path, enum dir_handling_type_e what, struct dir_handler_s *h, void *data)
{

    int ret             = 0;
    struct tar_ctx *ctx = (struct tar_ctx *)(data);
    char *pa            = (char *)calloc(1,strlen(path) + 1);
    char *fr            = (char *)calloc(1,strlen(h->root_catalog) + 1);

    strcpy(pa, path);
    strcpy(fr, h->root_catalog);

    char *sanitized = path_sanitize(fr, pa);

    switch (what) {
    case DirHandlingDir:
        ret = tar_catalog(ctx, path, sanitized);
        break;
    case DirHandlingFile:
        ret = tar_file(ctx, path, sanitized);
        break;
    default:
        ret = BackupErrorAny;
        break;
    }

    free(fr);
    free(pa);
    return ret;
}

bool backup_whole_directory(struct backup_handle_s *handle, trace_list_t *tl)
{
    bool success = true;

    trace_t *t = trace_append("backup_whole", tl, NULL,NULL);
    trace_t *t_tar = trace_append("tar", tl, NULL,NULL);
    struct dir_handler_s handle_walk;
    memset(&handle_walk, 0, sizeof handle_walk);
    unsigned int recursion_limit = 100;
    struct tar_ctx ctx;

    do {
        if (0 != tar_init(&ctx,t_tar, handle->backup_to, "w")) {
            trace_write(t, BackupErrorTar,0);
            success = false;
            break;
        }

        recursive_dir_walker_init(&handle_walk, tar_callback, &ctx);
        recursive_dir_walker(handle->backup_from, &handle_walk, &recursion_limit);
        recursive_dir_walker_deinit(&handle_walk);

        if (handle_walk.error) {
            trace_write(t, BackupErrorTar,0);
            success = false;
            break;
        }

        if (tar_deinit(&ctx) != 0) {
            trace_write(t, BackupErrorTar,0);
            success = false;
        }
        return success;

    } while (0);

    tar_deinit(&ctx);
    return success;
}
