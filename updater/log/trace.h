// Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
// For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

#pragma once

#include "error.h"

#define trace_start(trace_info) trace_start_impl(__func__, trace_info)

typedef struct trace_info_t {
    char *function_name;
    uint64_t start_time;
    uint64_t execution_time;
    update_error_t result;
} trace_info_t;

void trace_init();
void trace_deinit();
void trace_start_impl(const char *caller, trace_info_t *trace_info);
void trace_stop(trace_info_t *trace_info, enum e_ecoboot_update_code result, int extended_err_code);

void trace_set_error(trace_info_t *trace_info, enum e_ecoboot_update_code result, int extended_err_code);
void trace_print(enum log_level level, trace_info_t *trace_info);

