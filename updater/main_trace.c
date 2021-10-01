#include "main_trace.h"
#include <stdarg.h>
#include <common/enum_s.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

struct file_trace
{
    FILE *file;
};

static bool _trace_file_print(void *ptr, char *data, ...)
{
    struct file_trace *s = (struct file_trace *)ptr;
    va_list l;
    va_start(l, data);
    vfprintf(s->file, data, l);
    va_end(l);
    return true;
}

void main_status(trace_list_t *tl)
{
    const char *fname = "/user/updater.log";
    const size_t iobuf_size = 32768;
    trace_print(tl);
    struct file_trace t;
    t.file = fopen(fname, "w");
    if (t.file != NULL) {
        char* iobuf = malloc(iobuf_size);
        if(iobuf) {
            setvbuf(t.file, iobuf, _IOFBF, iobuf_size);
        }
        fprintf(t.file, "update status: %s\n", (trace_list_ok(tl)==true)?"OK":"FAIL");
        trace_dumps(tl, &t, _trace_file_print);
        printf("file saved %s", fname);
        fclose(t.file);
        free(iobuf);
    }
    else {
        printf("cant open file %s", fname);
    }
}

const char *strerror_main(int val)
{
    switch (val)
    {
        ENUMS(ErrMainOk);
        ENUMS(ErrMainVfs);
        ENUMS(ErrMainUpdate);
        ENUMS(ErrMainFactory);
        ENUMS(ErrMainRecovery);
        ENUMS(ErrNotHandled);
    }
    return "";
}

const char *strerror_main_ext(int val, int ext)
{
    switch (val)
    {
    case ErrMainOk:
        return "";
    case ErrMainVfs:
        return strerror(-ext);
    }
    return "";
}
