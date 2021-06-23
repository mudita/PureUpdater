#pragma once

/// KISS execution tracing capability
/// trace_list_t holds all traces added via trace_append
/// trace_t holds execution status appended by trace_append
/// whole can be logged via trace_print
/// - uses printf for trace_printf
/// - uses klib for list

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdbool.h"

    struct _trace
    {
        char *procedure_name;
        int err;
        int err_ext;
        const char *(*err_cstr)(int err);
        const char *(*err_ext_cstr)(int err, int err_ext);
        unsigned int line;
        const char *file;
        const char *opt_string;
    };

    typedef struct _trace trace_t;

    struct _trace_list
    {
        void *data;
    };
    typedef struct _trace_list trace_list_t;

    trace_list_t trace_init();
    void trace_deinit(trace_list_t *);
    void trace_print(trace_list_t *);
    trace_t *trace_append(const char *name,
                          trace_list_t *,
                          const char *(*err_cstr)(int err),
                          const char *(*err_ext_cstr)(int err, int ext_err));

    bool _trace_write(trace_t *, int err, int ext_err, unsigned int line, const char *file);
    // writes down first error - next errors: ignored
#define trace_write(trace, err, ext_err) _trace_write(trace, err, ext_err, __LINE__, __FILE__)
    void trace_printf(trace_t *, const char *txt);
    bool trace_ok(trace_t *t);
    bool trace_list_ok(trace_list_t *tl);

#ifdef __cplusplus
}
#endif
