/** System porting functions for the FF_FAT library
 */
#include <stdlib.h>

void *ff_memalloc(size_t n)
{
    return malloc(n);
}

void ff_memfree(void *ptr)
{
    free(ptr);
}