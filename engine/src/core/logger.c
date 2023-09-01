#include "logger.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

b8 initialize_logging() {
    // TODO: create log file
    return TRUE;
}

void shutdown_logging() {
    // TODO: create logging/write queued entries.

}


KAPI void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    
    // TODO: Handle errors and fatals differently
    // b8 is_error = level < 2;

    // 32K is just a large number that we hopefully don't hit
    // This is just faster than dynamic allocation
    char out_message[32000];
    memset(out_message, 0, sizeof(out_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, 32000, message, arg_ptr);
    va_end(arg_ptr);

    char out_message2[32000];
    sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

    // TODO: Temporary. We can do things 
    // platform specific in the future.
    printf("%s", out_message2);

    // TODO: Record Logging to a file.  
}