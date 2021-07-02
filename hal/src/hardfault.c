/** Hardfault simple fail support print data in the console
 */
#include <stdint.h>
#include <fsl_common.h>
#include <stdio.h>

enum fail_mode
{
    FAIL_USER = 1,
    FAIL_SYSTEM
};

//! Set position on stack
enum stk_regs
{
    stk_r0,
    stk_r1,
    stk_r2,
    stk_r3,
    stk_r12,
    stk_lr,
    stk_pc,
    stk_psr,
    stk_data //!User data on stack
};

static void crash(enum fail_mode mode, uintptr_t *sp)
{

    unsigned int stacked_r0;
    unsigned int stacked_r1;
    unsigned int stacked_r2;
    unsigned int stacked_r3;
    unsigned int stacked_r12;
    unsigned int stacked_lr;
    unsigned int stacked_pc;
    unsigned int stacked_psr;

    stacked_r0 = ((unsigned long)sp[0]);
    stacked_r1 = ((unsigned long)sp[1]);
    stacked_r2 = ((unsigned long)sp[2]);
    stacked_r3 = ((unsigned long)sp[3]);

    stacked_r12 = ((unsigned long)sp[4]);
    stacked_lr = ((unsigned long)sp[5]);
    stacked_pc = ((unsigned long)sp[6]);
    stacked_psr = ((unsigned long)sp[7]);

    printf("\n\n**** [Hard fault handler in the %s mode] ****\n", mode == FAIL_USER ? "user" : "system");
    printf("R0 = %08x\n", stacked_r0);
    printf("R1 = %08x\n", stacked_r1);
    printf("R2 = %08x\n", stacked_r2);
    printf("R3 = %08x\n", stacked_r3);
    printf("R12 = %08x\n", stacked_r12);
    printf("LR [R14] = %08x\n", stacked_lr);
    printf("PC [R15] = %08x\n", stacked_pc);
    printf("PSR = %08x\n", stacked_psr);
    printf("BFAR = %08x\n", (*((volatile uintptr_t *)(0xE000ED38))));
    printf("CFSR = %08x\n", (*((volatile uintptr_t *)(0xE000ED28))));
    printf("HFSR = %08x\n", (*((volatile uintptr_t *)(0xE000ED2C))));
    printf("DFSR = %08x\n", (*((volatile uintptr_t *)(0xE000ED30))));
    printf("AFSR = %08x\n", (*((volatile uintptr_t *)(0xE000ED3C))));
    printf("SCB_SHCSR = %08x\n", (uintptr_t)SCB->SHCSR);

#ifndef DEBUG
    for (volatile uint32_t i = 0; i < 100000000; ++i)
    {
    }
    NVIC_SystemReset();
#endif
    while (true)
    {
    }
}

void __attribute__((used)) HardFault_Handler(void)
{
    uintptr_t *sp;
    enum fail_mode cmode;
    asm(
        "TST LR, #4\n"
        "ITTEE EQ\n"
        "MRSEQ %[stackptr], MSP\n"
        "MOVEQ %[crashm],%[tsystem]\n"
        "MRSNE %[stackptr], PSP\n"
        "MOVNE %[crashm],%[tuser]\n"
        : [stackptr] "=r"(sp), [crashm] "=r"(cmode)
        :
        [tuser] "I"(FAIL_USER), [tsystem] "I"(FAIL_SYSTEM));
    crash(cmode, sp);
}