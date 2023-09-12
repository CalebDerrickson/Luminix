#include "logger.h"
#include "asserts.h"
#include "platform/platform.h"

// TODO: temporary
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// Implemantation of report_assertion_failure, defined in asserts.h
void report_assertion_failure(const char* expression, const char* message, const char* file, i32 line) {
    log_output(LOG_LEVEL_FATAL, "Assertion Failure: '%s', message: '%s', in file %s, line: %d\n", expression, message, file, line);
}

b8 initialize_logging() {
    // TODO: create log file
    return true;
}

void shutdown_logging() {
    // TODO: create logging/write queued entries.

}

void log_output(log_level level, const char* message, ...) {
    const char* level_strings[6] = {"[FATAL]: ", "[ERROR]: ", "[WARN]:  ", "[INFO]:  ", "[DEBUG]: ", "[TRACE]: "};
    
    // Handle errors and fatals differently
    b8 is_error = level < LOG_LEVEL_WARN;

    // 32K is just a large number that we hopefully don't hit
    // This is just faster than dynamic allocation
    const i32 msg_len = 32000;

    char out_message[msg_len];
    memset(out_message, 0, sizeof(out_message));

    // Format original message.
    // NOTE: Oddly enough, MS's headers override the GCC/Clang va_list type with a "typedef char* va_list" in some
    // cases, and as a result throws a strange error here. The workaround for now is to just use __builtin_va_list,
    // which is the type GCC/Clang's va_start expects.
    __builtin_va_list arg_ptr;
    va_start(arg_ptr, message);
    vsnprintf(out_message, msg_len, message, arg_ptr);
    va_end(arg_ptr);

    char out_message2[msg_len];
    sprintf(out_message2, "%s%s\n", level_strings[level], out_message);

    if (is_error){
        platform_console_write_error(out_message2, level);
    } else {
        platform_console_write(out_message2, level);
    }

    // TODO: Record Logging to a file.  
}