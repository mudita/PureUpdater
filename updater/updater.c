#include <console/console.h>

int main()
{
    debug_console_init();
    debug_console_write("Ala ma kota\r\n", 13);
    return 0;
}