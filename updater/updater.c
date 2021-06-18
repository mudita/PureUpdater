#include <hal/sysinit.h>
#include <hal/delay.h>
#include <stdio.h>

int main()
{   sysinit_setup();
    for(;;) {
        printf("Nowy lepszy sprzet %lu\r\n", get_jiffiess());
        msleep(1000);
    }
    return 0;
}