/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <fsl_common.h>

// Forward declaration

void _exit(int code);

// On Release, call the hardware reset procedure.
// On Debug we just enter an infinite loop, to be used as landmark when halting
// the debugger.
//
// It can be redefined in the application, if more functionality
// is required.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

void __attribute__((noreturn,used)) _exit(int code)
{
    printf("Program terminated by exit with code %i\n", code);
    if (code >= 0)
    {
        printf("Positive or success exit status - Reboot the system\n");
    }
    __asm volatile(
        "cpsid i\n"
        "dsb\n"
        "isb\n");
    if (code >= 0)
    {
        for (volatile uint32_t i = 0; i < 50000000; ++i)
        {
        }
        NVIC_SystemReset();
        while (1) {
          printf("waiting for reset");
          for (volatile uint32_t i = 0; i < 50000000; ++i) {}
        }
    }
    else
    {
        while (1)
        {
#ifndef DEBUG
            __asm volatile("wfi\n");
#endif
        }
    }
}

void abort(void)
{
    printf("***** FATAL: Abort called *****\n");
    __asm volatile(
        "cpsid i\n"
        "dsb\n"
        "isb\n");
#ifndef DEBUG
    for (volatile uint32_t i = 0; i < 100000000; ++i)
    {
    }
    NVIC_SystemReset();
#endif
    while (1)
    {
    }
}
#pragma GCC diagnostic pop
