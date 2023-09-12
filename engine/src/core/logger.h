#pragma once 
#include "defines.h"

// Details:
// LOG_LEVEL_FATAL denotes a critical failure in the engine and will cause the engine to crash 
// LOG_LEVEL_ERROR denotes an error has occured, the engine will still run, perhaps unpredictably
// LOG_LEVEL_WARN  tells us of a suboptimal event which has occured
// LOG_LEVEL_INFO  standard logging
// LOG_LEVEL_DEBUG verbose details of code execution
// LOG_LEVEL_TRACE far more verbose
//
// Both LOG_LEVEL_FATAL and LOG_LEVEL_ERROR are always on by default


#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disables debug and trace level logging on release
#if RELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif


typedef enum log_level{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4, 
    LOG_LEVEL_TRACE = 5
} log_level;

/**
 * @brief Initializes logging systems. Called twice; Once with state = 0 to get required memory size, 
 * then a second time passing allocated memory to state. 
 * 
 * @param memory_requirement A pointer to hold the required memory size of internal state.
 * @param state 0 if only requesting memory requirement; otherwise allocated memory block
 * @return b8 True if success; false otherwise
 */
b8 initialize_logging(u64* memory_requirement, void* state);
void shutdown_logging(void* state);

LAPI void log_output(log_level level, const char* message, ...);

#define LFATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef LERROR
#define LERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
#define LWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
#define LWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define LINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
#define LINFO(message, ...)
#endif


#if LOG_DEBUG_ENABLED == 1
#define LDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
#define LDEBUG(message, ...)
#endif


#if LOG_TRACE_ENABLED == 1
#define LTRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
#define LTRACE(message, ...)
#endif