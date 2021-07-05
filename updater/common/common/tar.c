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
#include "trace.h"

static void _autoclose(int *f)
{
    if (*f > 0) {
        close(*f);
    }
}

static void _autofree(char **f)
{
    free(*f);
}

#define AUTOCLOSE(var) int var __attribute__((__cleanup__(_autoclose)))
#define AUTOFREE(var) char* var __attribute__((__cleanup__(_autofree)))


int tar_init(struct tar_ctx *ctx, trace_t *t, const char *name, const char *operation_mode)
{
    memset(ctx, 0, sizeof(struct tar_ctx));
    ctx->size   = 1024 * 1024;
    ctx->buffer = calloc(1,ctx->size);
    ctx->t = t;

    int ret = mtar_open(&ctx->tar, name, operation_mode);
    if (ret != 0) {
        trace_write(ctx->t, ErrorTarLib, ret);
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

int tar_deinit(struct tar_ctx *ctx)
{
    int ret = 0;
    free(ctx->buffer);
    if (ctx->tar.stream) {
//        ret = mtar_finalize(&ctx->tar);
//        if (ret != 0) {
//            trace_write(ctx->t, ErrorTarLib, ret);
//        }

        ret = mtar_close(&ctx->tar);
        if (ret != 0) {
            trace_write(ctx->t, ErrorTarLib, ret);
            return ret;
        }
    }
    return 0;
}

int tar_catalog(struct tar_ctx *ctx, const char *sanitized)
{
    if (sanitized == NULL) {
        return 0;
    }
    int ret = mtar_write_dir_header(&(*ctx).tar, sanitized);
    if (ret != 0) {
        trace_write(ctx->t, ErrorTarLib, ret);
    }
    return ret;
}

int tar_file(struct tar_ctx *ctx, const char *path, const char *sanitized)
{
    int ret            = 0;
    off_t file_size    = 0;
    off_t yet_to_write = 0;
    int bytes_read     = 0;

    printf("Try tar file: %s to: %s\n", path, sanitized);

    AUTOCLOSE(f) = open(path, O_RDONLY);
    if (f <= 0) {
        trace_write(ctx->t, ErrorTarStd, errno);
        goto exit;
    }

    struct stat buf;
    ret = fstat(f, &buf);
    if (ret != 0) {
        trace_write(ctx->t, ErrorTarStd, errno);
        goto exit;
    }

    file_size    = buf.st_size;
    yet_to_write = buf.st_size;

    ret = mtar_write_file_header(&(*ctx).tar, sanitized, file_size);
    if (ret != 0) {
        trace_write(ctx->t, ErrorTarLib, ret);
        goto exit;
    }

    do {
        memset(ctx->buffer, 0, ctx->size);
        bytes_read = read(f, ctx->buffer, ctx->size);
        ret        = mtar_write_data(&(ctx->tar), ctx->buffer, bytes_read);
        if (ret != 0) {
            trace_write(ctx->t, ErrorTarLib, ret);
            goto exit;
        }
        if (yet_to_write < bytes_read) {
            trace_write(ctx->t, ErrorTarAny, 0);
            ret            = -1;
            goto exit;
            ret = f;
        }
        yet_to_write -= bytes_read;
    } while (bytes_read > 0 && yet_to_write);

    if (ret != 0) {
        goto exit;
    }

exit:
    return ret;
}

// TODO check path with double // if requires sanitization
// TODO check path with ./ and /
int un_tar_file(struct tar_ctx *ctx, mtar_header_t *header, const char *where)
{
    int ret              = 0;
    int bytes_read       = 0;
    size_t yet_to_write = header->size;
    AUTOFREE(out)        = calloc(1, strlen(header->name) + strlen(where) + 2);
    sprintf(out, "%s/%s", where, header->name);

    AUTOCLOSE(f) = open(out, O_WRONLY|O_CREAT);
    if (f <= 0) {
        trace_write(ctx->t, ErrorTarStd, errno);
        trace_printf(ctx->t, out);
        ret = f;
        goto exit;
    }

    while(ctx->tar.remaining_data)
    {
         size_t read_size = yet_to_write > ctx->size ? ctx->size : yet_to_write;
         memset(ctx->buffer, 0, ctx->size);
         ret = mtar_read_data(&ctx->tar, ctx->buffer, read_size);
         if (ret != 0) {
             trace_write(ctx->t, ErrorTarLib, bytes_read);
             goto exit;
         }
 
         ret = write(f, ctx->buffer, read_size);
         if (0 > ret) {
             trace_write(ctx->t, ErrorTarStd, errno);
             goto exit;
         }
     }

exit:
    return ret;
}

int un_tar_catalog(struct tar_ctx *ctx, mtar_header_t *header, const char *where)
{
    int ret       = 0;
    ssize_t header_name_len = strlen(header->name);

    if (strlen(header->name) == 0)
    {
        return 0;
    }

    if (header_name_len == 2 
            && (  strncmp(header->name, "./", header_name_len) == 0
               || strncmp(header->name, "..", header_name_len) == 0)) {
        return 0;
    }


    // TODO is it needed? remove_here(header->name);
    AUTOFREE(out) = calloc(1, strlen(header->name) + strlen(where) + 2);
    sprintf(out, "%s/%s", where, header->name);
    // TODO is it needed? remove_dup_slash(out);

    struct stat data;
    ret = stat(out, &data);
    if (ret != 0 && errno == ENOENT) {
        ret = 0;
    }
    if (ret != 0 && errno != 0) {
        trace_write(ctx->t, ErrorTarStd, errno);
        goto exit;
    }

        ret = mkdir(out, S_IRWXU | S_IXOTH);
        if (ret != 0 && errno == EEXIST) {
            ret = 0;
        }
        if (ret != 0) {
            trace_write(ctx->t, ErrorTarStd, errno);
            goto exit;
        }

exit:
    return ret;
}

int tar_next(struct tar_ctx * ctx)
{
    int lib_error = mtar_next(&ctx->tar);
    if (lib_error) {
        trace_write(ctx->t, ErrorTarLib, lib_error);
        return ErrorTarStd;
    }
    return ErrorTarOk;
}

const char *tar_strerror(int err)
{
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

const char *tar_strerror_ext(int err, int ext_err)
{
    switch (err) {
    case ErrorTarStd:
        return strerror(ext_err);
    case ErrorTarLib:
        return mtar_strerror(ext_err);
    }
    return "";
}
