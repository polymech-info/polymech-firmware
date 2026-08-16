#ifndef APP_LOGGER_H
#define APP_LOGGER_H

#include <components/Logger.h>

#include "./config.h"


#if defined(ENABLE_LOGGER)
  extern Logger* g_logger;  
  // For use inside a Component class method (implicitly uses 'this')
  #define L_FATAL(format, ...)   if (g_logger) g_logger->fatal(this, format, ##__VA_ARGS__)
  #define L_ERROR(format, ...)   if (g_logger) g_logger->error(this, format, ##__VA_ARGS__)
  #define L_WARN(format, ...)    if (g_logger) g_logger->warn(this, format, ##__VA_ARGS__)
  #define L_INFO(format, ...)    if (g_logger) g_logger->info(this, format, ##__VA_ARGS__)
  #define L_TRACE(format, ...)   if (g_logger) g_logger->trace(this, format, ##__VA_ARGS__)
  #define L_VERBOSE(format, ...) if (g_logger) g_logger->verbose(this, format, ##__VA_ARGS__)

  // For static calls (no 'this' available)
  #define LS_FATAL(format, ...)   if (g_logger) g_logger->fatal(format, ##__VA_ARGS__)
  #define LS_ERROR(format, ...)   if (g_logger) g_logger->error(format, ##__VA_ARGS__)
  #define LS_WARN(format, ...)    if (g_logger) g_logger->warn(format, ##__VA_ARGS__)
  #define LS_INFO(format, ...)    if (g_logger) g_logger->info(format, ##__VA_ARGS__)
  #define LS_TRACE(format, ...)   if (g_logger) g_logger->trace(format, ##__VA_ARGS__)
  #define LS_VERBOSE(format, ...) if (g_logger) g_logger->verbose(format, ##__VA_ARGS__)
#else
  #define L_FATAL(format, ...) Log.fatal(format, ##__VA_ARGS__)
  #define L_ERROR(format, ...) Log.error(format, ##__VA_ARGS__)
  #define L_WARN(format, ...)  Log.warning(format, ##__VA_ARGS__)
  #define L_INFO(format, ...)  Log.info(format, ##__VA_ARGS__)
  #define L_TRACE(format, ...) Log.trace(format, ##__VA_ARGS__)
  #define L_VERBOSE(format, ...) Log.verbose(format, ##__VA_ARGS__)

  #define LS_FATAL(format, ...) Log.fatal(format, ##__VA_ARGS__)
  #define LS_ERROR(format, ...) Log.error(format, ##__VA_ARGS__)
  #define LS_WARN(format, ...)  Log.warning(format, ##__VA_ARGS__)
  #define LS_INFO(format, ...)  Log.info(format, ##__VA_ARGS__)
  #define LS_TRACE(format, ...) Log.trace(format, ##__VA_ARGS__)
  #define LS_VERBOSE(format, ...) Log.verbose(format, ##__VA_ARGS__)
#endif


/*
// Uncomment to enable automatic file/line info in all L_* logging calls
// #define ENABLE_LOGGING_FILEPATH

#ifdef DISABLE_LOGGING
  // When logging is completely disabled, all logging macros become no-ops
  #define L_FATAL_BASE(format, ...)
  #define L_ERROR_BASE(format, ...)
  #define L_WARN_BASE(format, ...)
  #define L_INFO_BASE(format, ...)
  #define L_TRACE_BASE(format, ...)
  #define L_VERBOSE_BASE(format, ...)

  #define LS_FATAL_BASE(format, ...)
  #define LS_ERROR_BASE(format, ...)
  #define LS_WARN_BASE(format, ...)
  #define LS_INFO_BASE(format, ...)
  #define LS_TRACE_BASE(format, ...)
  #define LS_VERBOSE_BASE(format, ...)

  #define L_FATAL_LOC(format, ...)
  #define L_ERROR_LOC(format, ...)
  #define L_WARN_LOC(format, ...)
  #define L_INFO_LOC(format, ...)
  #define L_TRACE_LOC(format, ...)
  #define L_VERBOSE_LOC(format, ...)

  #define LS_FATAL_LOC(format, ...)
  #define LS_ERROR_LOC(format, ...)
  #define LS_WARN_LOC(format, ...)
  #define LS_INFO_LOC(format, ...)
  #define LS_TRACE_LOC(format, ...)
  #define LS_VERBOSE_LOC(format, ...)

  #define L_FATAL(format, ...)
  #define L_ERROR(format, ...)
  #define L_WARN(format, ...)
  #define L_INFO(format, ...)
  #define L_TRACE(format, ...)
  #define L_VERBOSE(format, ...)

  #define LS_FATAL(format, ...)
  #define LS_ERROR(format, ...)
  #define LS_WARN(format, ...)
  #define LS_INFO(format, ...)
  #define LS_TRACE(format, ...)
  #define LS_VERBOSE(format, ...)

#elif defined(ENABLE_LOGGER)
  extern Logger* g_logger;
  
  // Base logging macros - always available
  #define L_FATAL_BASE(format, ...)   if (g_logger) g_logger->fatal(this, format, ##__VA_ARGS__)
  #define L_ERROR_BASE(format, ...)   if (g_logger) g_logger->error(this, format, ##__VA_ARGS__)
  #define L_WARN_BASE(format, ...)    if (g_logger) g_logger->warn(this, format, ##__VA_ARGS__)
  #define L_INFO_BASE(format, ...)    if (g_logger) g_logger->info(this, format, ##__VA_ARGS__)
  #define L_TRACE_BASE(format, ...)   if (g_logger) g_logger->trace(this, format, ##__VA_ARGS__)
  #define L_VERBOSE_BASE(format, ...) if (g_logger) g_logger->verbose(this, format, ##__VA_ARGS__)

  #define LS_FATAL_BASE(format, ...)   if (g_logger) g_logger->fatal(format, ##__VA_ARGS__)
  #define LS_ERROR_BASE(format, ...)   if (g_logger) g_logger->error(format, ##__VA_ARGS__)
  #define LS_WARN_BASE(format, ...)    if (g_logger) g_logger->warn(format, ##__VA_ARGS__)
  #define LS_INFO_BASE(format, ...)    if (g_logger) g_logger->info(format, ##__VA_ARGS__)
  #define LS_TRACE_BASE(format, ...)   if (g_logger) g_logger->trace(format, ##__VA_ARGS__)
  #define LS_VERBOSE_BASE(format, ...) if (g_logger) g_logger->verbose(format, ##__VA_ARGS__)

  // Location-aware macros - always available  
  #define L_FATAL_LOC(format, ...)   L_FATAL_BASE(format, ##__VA_ARGS__)
  #define L_ERROR_LOC(format, ...)   L_ERROR_BASE(format, ##__VA_ARGS__)
  #define L_WARN_LOC(format, ...)    L_WARN_BASE(format, ##__VA_ARGS__)
  #define L_INFO_LOC(format, ...)    L_INFO_BASE(format, ##__VA_ARGS__)
  #define L_TRACE_LOC(format, ...)   L_TRACE_BASE(format, ##__VA_ARGS__)
  #define L_VERBOSE_LOC(format, ...) L_VERBOSE_BASE(format, ##__VA_ARGS__)

  #define LS_FATAL_LOC(format, ...)   LS_FATAL_BASE(format, ##__VA_ARGS__)
  #define LS_ERROR_LOC(format, ...)   LS_ERROR_BASE(format, ##__VA_ARGS__)
  #define LS_WARN_LOC(format, ...)    LS_WARN_BASE(format, ##__VA_ARGS__)
  #define LS_INFO_LOC(format, ...)    LS_INFO_BASE(format, ##__VA_ARGS__)
  #define LS_TRACE_LOC(format, ...)   LS_TRACE_BASE(format, ##__VA_ARGS__)
  #define LS_VERBOSE_LOC(format, ...) LS_VERBOSE_BASE(format, ##__VA_ARGS__)

  // Standard macros - conditional on ENABLE_LOGGING_FILEPATH
  #ifdef ENABLE_LOGGING_FILEPATH
    #define L_FATAL(format, ...)   L_FATAL_LOC(format, ##__VA_ARGS__)
    #define L_ERROR(format, ...)   L_ERROR_LOC(format, ##__VA_ARGS__)
    #define L_WARN(format, ...)    L_WARN_LOC(format, ##__VA_ARGS__)
    #define L_INFO(format, ...)    L_INFO_LOC(format, ##__VA_ARGS__)
    #define L_TRACE(format, ...)   L_TRACE_LOC(format, ##__VA_ARGS__)
    #define L_VERBOSE(format, ...) L_VERBOSE_LOC(format, ##__VA_ARGS__)

    #define LS_FATAL(format, ...)   LS_FATAL_LOC(format, ##__VA_ARGS__)
    #define LS_ERROR(format, ...)   LS_ERROR_LOC(format, ##__VA_ARGS__)
    #define LS_WARN(format, ...)    LS_WARN_LOC(format, ##__VA_ARGS__)
    #define LS_INFO(format, ...)    LS_INFO_LOC(format, ##__VA_ARGS__)
    #define LS_TRACE(format, ...)   LS_TRACE_LOC(format, ##__VA_ARGS__)
    #define LS_VERBOSE(format, ...) LS_VERBOSE_LOC(format, ##__VA_ARGS__)
  #else
    #define L_FATAL(format, ...)   L_FATAL_BASE(format, ##__VA_ARGS__)
    #define L_ERROR(format, ...)   L_ERROR_BASE(format, ##__VA_ARGS__)
    #define L_WARN(format, ...)    L_WARN_BASE(format, ##__VA_ARGS__)
    #define L_INFO(format, ...)    L_INFO_BASE(format, ##__VA_ARGS__)
    #define L_TRACE(format, ...)   L_TRACE_BASE(format, ##__VA_ARGS__)
    #define L_VERBOSE(format, ...) L_VERBOSE_BASE(format, ##__VA_ARGS__)

    #define LS_FATAL(format, ...)   LS_FATAL_BASE(format, ##__VA_ARGS__)
    #define LS_ERROR(format, ...)   LS_ERROR_BASE(format, ##__VA_ARGS__)
    #define LS_WARN(format, ...)    LS_WARN_BASE(format, ##__VA_ARGS__)
    #define LS_INFO(format, ...)    LS_INFO_BASE(format, ##__VA_ARGS__)
    #define LS_TRACE(format, ...)   LS_TRACE_BASE(format, ##__VA_ARGS__)
    #define LS_VERBOSE(format, ...) LS_VERBOSE_BASE(format, ##__VA_ARGS__)
  #endif
#else
  // Fallback to Arduino Log when ENABLE_LOGGER is not defined
  #define L_FATAL_BASE(format, ...) Log.fatal(format, ##__VA_ARGS__)
  #define L_ERROR_BASE(format, ...) Log.error(format, ##__VA_ARGS__)
  #define L_WARN_BASE(format, ...)  Log.warning(format, ##__VA_ARGS__)
  #define L_INFO_BASE(format, ...)  Log.info(format, ##__VA_ARGS__)
  #define L_TRACE_BASE(format, ...) Log.trace(format, ##__VA_ARGS__)
  #define L_VERBOSE_BASE(format, ...) Log.verbose(format, ##__VA_ARGS__)

  #define L_FATAL_LOC(format, ...)   Log.fatal(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)
  #define L_ERROR_LOC(format, ...)   Log.error(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)
  #define L_WARN_LOC(format, ...)    Log.warning(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)
  #define L_INFO_LOC(format, ...)    Log.info(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)
  #define L_TRACE_LOC(format, ...)   Log.trace(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)
  #define L_VERBOSE_LOC(format, ...) Log.verbose(format " [%s:%d]", ##__VA_ARGS__, __FILE__, __LINE__)

  // Standard macros - conditional on ENABLE_LOGGING_FILEPATH
  #ifdef ENABLE_LOGGING_FILEPATH
    #define L_FATAL(format, ...)   L_FATAL_LOC(format, ##__VA_ARGS__)
    #define L_ERROR(format, ...)   L_ERROR_LOC(format, ##__VA_ARGS__)
    #define L_WARN(format, ...)    L_WARN_LOC(format, ##__VA_ARGS__)
    #define L_INFO(format, ...)    L_INFO_LOC(format, ##__VA_ARGS__)
    #define L_TRACE(format, ...)   L_TRACE_LOC(format, ##__VA_ARGS__)
    #define L_VERBOSE(format, ...) L_VERBOSE_LOC(format, ##__VA_ARGS__)
  #else
    #define L_FATAL(format, ...)   L_FATAL_BASE(format, ##__VA_ARGS__)
    #define L_ERROR(format, ...)   L_ERROR_BASE(format, ##__VA_ARGS__)
    #define L_WARN(format, ...)    L_WARN_BASE(format, ##__VA_ARGS__)
    #define L_INFO(format, ...)    L_INFO_BASE(format, ##__VA_ARGS__)
    #define L_TRACE(format, ...)   L_TRACE_BASE(format, ##__VA_ARGS__)
    #define L_VERBOSE(format, ...) L_VERBOSE_BASE(format, ##__VA_ARGS__)
  #endif

  // Static versions (same as non-static when using Arduino Log)
  #define LS_FATAL(format, ...)     L_FATAL(format, ##__VA_ARGS__)
  #define LS_ERROR(format, ...)     L_ERROR(format, ##__VA_ARGS__)
  #define LS_WARN(format, ...)      L_WARN(format, ##__VA_ARGS__)
  #define LS_INFO(format, ...)      L_INFO(format, ##__VA_ARGS__)
  #define LS_TRACE(format, ...)     L_TRACE(format, ##__VA_ARGS__)
  #define LS_VERBOSE(format, ...)   L_VERBOSE(format, ##__VA_ARGS__)

  #define LS_FATAL_LOC(format, ...)   L_FATAL_LOC(format, ##__VA_ARGS__)
  #define LS_ERROR_LOC(format, ...)   L_ERROR_LOC(format, ##__VA_ARGS__)
  #define LS_WARN_LOC(format, ...)    L_WARN_LOC(format, ##__VA_ARGS__)
  #define LS_INFO_LOC(format, ...)    L_INFO_LOC(format, ##__VA_ARGS__)
  #define LS_TRACE_LOC(format, ...)   L_TRACE_LOC(format, ##__VA_ARGS__)
  #define LS_VERBOSE_LOC(format, ...) L_VERBOSE_LOC(format, ##__VA_ARGS__)
#endif
*/
#endif