#include "log.h"

const char *log_filename = "/user/updater.log";
const unsigned int buffer_size = 512;

void debug_log_impl(const char *file, const char *func, int line, const char *msg, ...) {
    char str[buffer_size];
    int length = -1;
    va_list argList;
    va_start(argList, msg);
    length = vsnprintf(str, sizeof(str), msg, argList);
    va_end(argList);
    if (length > 0) {
        printf("([%s:%d] %s): %s\n", file, line, func, str);
        FILE *log_file = fopen(log_filename, "a");
        if (log_file != NULL) {
            fprintf(log_file, "([%s:%d] %s): %s\n", file, line, func, str);
            fclose(log_file);
        } else {
            printf("Failed to write to file!\n");
        }
    }
}