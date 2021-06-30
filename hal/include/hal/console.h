#pragma once

#include <stddef.h>

/** Initialize debug console 
 * @return Error code 
*/
int debug_console_init(void);

/** Log to the debug output channels
 */
int debug_console_write(const char *buffer, size_t len);

