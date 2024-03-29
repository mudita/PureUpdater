#include <errno.h>
#include <microtar/microtar.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include "path_opts.h"
#include "tar.h"
#include "log.h"

#ifdef DEBUG_UNPACK
#define log_unpack(...) printf(__VA_ARGS__)
#else
#define log_unpack(...)
#endif

static void _autoclose(int *f) {
    if (*f > 0) {
        close(*f);
    }
}

static void _autofree(char **f) {
    free(*f);
}

#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))
#define AUTOFREE(var) char* var __attribute__((__cleanup__(_autofree)))


int tar_init(struct tar_ctx *ctx, const char *name, const char *operation_mode) {
    memset(ctx, 0, sizeof(struct tar_ctx));
    ctx->size = 1024 * 1024;
    ctx->buffer = calloc(1, ctx->size);

    int ret = mtar_open(&ctx->tar, name, operation_mode);
    if (ret != 0) {
        debug_log("Tar: unable to open tar archive: %s in mode %s: %d", name, operation_mode, ret);
        return ret;
    }

// TODO: implement some tar header rewind to use flush with: mtar_finalize
//    if (operation_mode != NULL && operation_mode[0] == 'a') {
//        printf("-------------------- pos %lu\n",ftell(ctx->tar.stream));
//        fseek( ctx->tar.stream, -1024, SEEK_CUR);
//        printf("-------------------- pos %lu\n",ftell(ctx->tar.stream));
//    }

    return ret;
}

int tar_deinit(struct tar_ctx *ctx) {
    int ret = 0;
    free(ctx->buffer);
    if (ctx->tar.stream) {
//        ret = mtar_finalize(&ctx->tar);
//        if (ret != 0) {
//            trace_write(ctx->t, ErrorTarLib, ret);
//        }

        ret = mtar_close(&ctx->tar);
        if (ret != 0) {
            debug_log("Tar: unable to close archive: %d", ret);
            return ret;
        }
    }
    return 0;
}

int tar_catalog(struct tar_ctx *ctx, const char *sanitized_name) {
    if (sanitized_name == NULL) {
        return 0;
    }
    int ret = mtar_write_dir_header(&(*ctx).tar, sanitized_name);
    if (ret != 0) {
        debug_log("Tar: unable to write dir header for file %s : %d", sanitized_name, ret);
    }
    return ret;
}

int tar_file(struct tar_ctx *ctx, const char *path, const char *sanitized_name) {
    int ret = 0;
    off_t file_size = 0;
    off_t yet_to_write = 0;
    int bytes_read = 0;

    debug_log("Tar: appending file %s", path);

    AUTOCLOSE(f) = open(path, O_RDONLY);
    if (f <= 0 && errno == ENOENT) {
        debug_log("Tar: ignored non existing file: %s", path);
        ret = 0;
        goto exit;
    }
    if (f <= 0) {
        debug_log("Tar: failed to open file: %s", sanitized_name);
        ret = ErrorTarStd;
        goto exit;
    }

    struct stat buf;
    ret = stat(path, &buf);
    if (ret != 0) {
        debug_log("Tar: can't get stat info from file: %s : %d", sanitized_name, ret);
        ret = ErrorTarStd;
        goto exit;
    }

    file_size = buf.st_size;
    yet_to_write = buf.st_size;

    ret = mtar_write_file_header(&(*ctx).tar, sanitized_name, file_size);
    if (ret != 0) {
        debug_log("Tar: unable to write file header for file %s, file size: %d : %d", sanitized_name, file_size, ret);
        ret = ErrorTarLib;
        goto exit;
    }

    do {
        memset(ctx->buffer, 0, ctx->size);
        bytes_read = read(f, ctx->buffer, ctx->size);
        ret = mtar_write_data(&(ctx->tar), ctx->buffer, bytes_read);
        if (ret != 0) {
            debug_log("Tar: data write (%d bytes) to archive failed: %d", bytes_read, ret);
            ret = ErrorTarLib;
            goto exit;
        }
        if (yet_to_write < bytes_read) {
            debug_log("Tar: remaining data size is lower than read");
            ret = ErrorTarLib;
            goto exit;
        }
        yet_to_write -= bytes_read;
    } while (bytes_read > 0 && yet_to_write);

    exit:
    return ret;
}

// TODO check path with double // if requires sanitization
// TODO check path with ./ and /
int un_tar_file(struct tar_ctx *ctx, mtar_header_t *header, const char *where) {
    int ret = 0;
    char *name = header->name;
    size_t yet_to_write = header->size;

    path_remove_cwd(name);
    AUTOFREE(out) = calloc(1, strlen(name) + strlen(where) + 2);
    sprintf(out, "%s/%s", where, name);

    debug_log("Tar: unpacking file (%d.%dkb) to %s", header->size / 1024, header->size % 1024, out);

    AUTOCLOSE(f) = open(out, O_WRONLY | O_CREAT);
    if (f <= 0) {
        debug_log("Tar: failed to open archive: %s : %d", out, f);
        ret = f;
        goto exit;
    }

    do {
        size_t read_size = yet_to_write > ctx->size ? ctx->size : yet_to_write;
        memset(ctx->buffer, 0, ctx->size);
        ret = mtar_read_data(&ctx->tar, ctx->buffer, read_size);
        if (ret != 0) {
            debug_log("Tar: failed to read data (%d bytes) from archive: %d", read_size, ret);
            goto exit;
        }

        ret = write(f, ctx->buffer, read_size);
        if (0 > ret) {
            debug_log("Tar: failed to write file (%d bytes) from archive to disk: %d", ret);
            goto exit;
        } else {
            ret = 0;
        }
        yet_to_write = ctx->tar.remaining_data;
    } while (yet_to_write);

    exit:
    return ret;
}

int un_tar_catalog(struct tar_ctx *ctx, mtar_header_t *header, const char *where) {
    (void) ctx;
    int ret = 0;
    ssize_t header_name_len = strlen(header->name);
    char *name = header->name;

    if (strlen(header->name) == 0) {
        return 0;
    }

    if (header_name_len == 2
        && (strncmp(header->name, "./", header_name_len) == 0
            || strncmp(header->name, "..", header_name_len) == 0)) {
        return 0;
    }

    path_remove_cwd(name);

    AUTOFREE(out) = calloc(1, strlen(name) + strlen(where) + 2);
    sprintf(out, "%s/%s", where, name);
    path_remove_trailing_slash(out);

    struct stat data;
    ret = stat(out, &data);
    if (ret != 0 && errno == ENOENT) {
        debug_log("Tar: path %s not found: %d", out, ret);
        ret = 0;
    }
    if (ret != 0 && errno != 0) {
        debug_log("Tar: path stat info error: %d:%d", ret, errno);
        goto exit;
    }

    debug_log("Tar: creating path %s", out);
    ret = mkdir(out, S_IRWXU | S_IXOTH);
    if (ret != 0 && errno == EEXIST) {
        debug_log("Tar: path %s already exists", out);
        ret = 0;
    }
    if (ret != 0) {
        debug_log("Tar: can't create path: %d", ret);
        goto exit;
    }

    exit:
    return ret;
}

int tar_next(struct tar_ctx *ctx) {
    int lib_error = mtar_next(&ctx->tar);
    if (lib_error) {
        debug_log("Tar: can't move on to next file: %d", lib_error);
        return ErrorTarStd;
    }
    return ErrorTarOk;
}

const char *tar_strerror(int err) {
    switch (err) {
        case ErrorTarOk:
            return "ErrorTarOk";
        case ErrorTarAny:
            return "ErrorTarAny";
        case ErrorTarStd:
            return "ErrorTarStd";
        case ErrorTarLib:
            return "ErrorTarLib";
    }
    return "";
}

const char *tar_strerror_ext(int err, int ext_err) {
    switch (err) {
        case ErrorTarStd:
            return strerror(ext_err);
        case ErrorTarLib:
            return mtar_strerror(ext_err);
    }
    return "";
}
