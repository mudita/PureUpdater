#include <klib/klist.h>
#include <stdio.h>
#include <string.h>
#include "trace.h"

void on_free_trace_t(void *data)
{
    trace_t *trace = (trace_t *)data;
    if (trace->procedure_name)
        free(trace->procedure_name);
    if (trace->opt_string)
        free((char *)trace->opt_string);
}

KLIST_INIT(trace_list, trace_t, on_free_trace_t);

trace_list_t trace_init()
{
    trace_list_t tl;
    tl.data = kl_init(trace_list);
    return tl;
}

void trace_deinit(trace_list_t *tl)
{
    kl_destroy_trace_list(tl->data);
}

void trace_print(trace_list_t *tl)
{
    kliter_t(trace_list) *begin = kl_begin((kl_trace_list_t *)(tl->data));
    kliter_t(trace_list) *end   = kl_end((kl_trace_list_t *)(tl->data));
    int i                       = 0;
    while (begin != end && begin != NULL) {
        trace_t *t = &begin->data;
        if (t != NULL) {
            if (trace_ok(t)) {
                printf("t:%d,name:\"%s\"", i, t->procedure_name);
            }
            else {
                printf("t:%d,name:\"%s\",err:%d", i, t->procedure_name, t->err);
                if (t->err_ext) {
                    printf(",ext_err:%d", t->err_ext);
                }
                if (NULL != t->err_cstr) {
                    printf(",err_cstr:\"%s\"", t->err_cstr(t->err));
                }
                if (NULL != t->err_ext_cstr) {
                    printf(",ext_err_cstr:\"%s\"", t->err_ext_cstr(t->err, t->err_ext));
                }
                printf(",file:\"%s\",line:%d", t->file, t->line);
                if (NULL != t->opt_string) {
                    printf(",opt:%s", t->opt_string);
                }
            }
            printf("%s", "\n");
            ++i;
        }
        begin = kl_next(begin);
    }
}

trace_t *trace_append(const char *name,
                      trace_list_t *tl,
                      const char *(*err_cstr)(int err),
                      const char *(*err_ext_cstr)(int err, int ext_err))
{
    trace_t *t        = kl_pushp(trace_list, tl->data);
    t->procedure_name = calloc(1, strlen(name) + 1);
    sprintf(t->procedure_name, "%s", name);
    t->err_cstr     = err_cstr;
    t->err_ext_cstr = err_ext_cstr;
    return t;
}

bool _trace_write(trace_t *t, int err, int ext_err, unsigned int line, const char *file)
{
    if (trace_ok(t)) {
        t->err     = err;
        t->err_ext = ext_err;
        t->line    = line;
        t->file    = file;
    }

    return t->err != 0;
}

void trace_printf(trace_t *t, const char *txt)
{
    t->opt_string = calloc(1, strlen(txt) + 1);
    strcpy((char *)t->opt_string, txt);
}

bool trace_ok(trace_t *t)
{
    return t->err == 0;
}

bool trace_list_ok(trace_list_t *tl)
{
    bool ret                    = true;
    kliter_t(trace_list) *begin = kl_begin((kl_trace_list_t *)(tl->data));
    kliter_t(trace_list) *end   = kl_end((kl_trace_list_t *)(tl->data));
    while (begin != end && begin != NULL) {
        trace_t *t = &begin->data;
        if (!(trace_ok(t))) {
            ret = false;
            break;
        }
        begin = kl_next(begin);
    }
    return ret;
}
