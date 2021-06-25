// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#include <stdio.h>
#include <time.h>
#include "log.h"
#include "trace.h"

const char* trace_file_name = "boottrace.log";
static FILE *fp_trace;

void trace_init() {
    fp_trace = fopen(trace_file_name, "w");
}

void trace_deinit() {
    if (fp_trace != NULL) {
        fclose(fp_trace);
    }
}

void trace_start_impl(const char *caller, trace_info_t *trace_info) {
    if (trace_info != NULL) {
        trace_info->function_name = caller;
        trace_info->start_time = time(NULL);
        trace_info->execution_time = 0;
        trace_info->result.result = None;
        trace_info->result.extended_err_code = 0;
        trace_print(LOG_INFO, trace_info);

        if (fp_trace != NULL) {
            fprintf(fp_trace, "start %lu %s %p\n", trace_info->start_time, trace_info->function_name,
                    trace_info->function_name);
        }
    }
}

void trace_stop(trace_info_t *trace_info, enum e_ecoboot_update_code result, int extended_err_code) {
    if (trace_info != NULL) {
        trace_info->execution_time = time(NULL) - trace_info->start_time;
        trace_print(LOG_INFO, trace_info);

        if (fp_trace != NULL) {
            fprintf(fp_trace, "exit %s %p %lu\n", trace_info->function_name, trace_info->function_name, time(NULL));
        }
    }
}

void trace_set_error(trace_info_t *trace_info, enum e_ecoboot_update_code result, int extended_err_code) {
    if (trace_info != NULL) {
        trace_info->result.result = result;
    }
}

void trace_print(enum log_level level, trace_info_t *trace_info) {
    LOG(level, "Function: %s", trace_info->function_name);
    LOG(level, "Exec time: %lu", trace_info->execution_time);
    LOG(level, "Result: %d", trace_info->result.result);
    LOG(level, "Ext code: %d", trace_info->result.extended_err_code);
}
