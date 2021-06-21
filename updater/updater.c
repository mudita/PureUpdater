#include <hal/sysinit.h>
#include <hal/delay.h>
#include <hal/emmc.h>
#include <stdio.h>


int main()
{   // System initialize
    sysinit_setup();
    // Main loop
    printf("Ta karta EMMC chyba dziala%p\n", emmc_card());
    for(;;) {
        printf("test lepszego sprzetu %lu\n", get_jiffiess());
        msleep(1000);
    }
    return 0;
}
