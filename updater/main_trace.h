#pragma once 

#include <common/trace.h>

enum
{
    ErrMainOk,
    ErrMainVfs,
    ErrMainUpdate,
    ErrMainFactory,
    ErrMainRecovery,
    ErrNotHandled,
};

const char *strerror_main(int val);
const char *strerror_main_ext(int val, int ext);
/// print data from tl to file
void main_status(trace_list_t *tl);
