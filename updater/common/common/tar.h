#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#include <microtar/microtar.h>
#include "trace.h"

enum tar_error_e
{
    ErrorTarOk,
    ErrorTarAny,
    ErrorTarStd,
    ErrorTarLib,
};

struct tar_ctx
{
    mtar_t tar;
    void *buffer;
    size_t size;
    trace_t *t;
};

int tar_init(struct tar_ctx *ctx, trace_t *t, const char* name, const char* operation_mode);
int tar_deinit(struct tar_ctx *ctx);
/// append file to opened tar
int tar_file(struct tar_ctx *ctx, const char *path, const char *sanitized);
/// append catalog to opened tar
int tar_catalog(struct tar_ctx *ctx, const char *sanitized);

/// overwrite file if exists
int un_tar_file(struct tar_ctx *ctx, mtar_header_t *header, const char *where);
/// create catalog if not exists
int un_tar_catalog(struct tar_ctx *ctx, mtar_header_t *header, const char *where);

int tar_next(struct tar_ctx*);

const char* tar_strerror(int err);
const char* tar_strerror_ext(int err, int ext_err);

#ifdef __cplusplus
extern "C"
}
#endif
