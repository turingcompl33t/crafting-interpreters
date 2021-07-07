/**
 * @file common.h
 */

#ifndef CLOX_COMMON_H
#define CLOX_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/** Denotes whether execution tracing is enabled */
// #define DEBUG_TRACE_EXECUTION

/** Denotes whether the contents of the chunk are printed after compilation */
// #define DEBUG_PRINT_CODE

/** Denotes whether the GC is run deterministically (as often as possible) */
#define DEBUG_STRESS_GC

/** Denotes whether GC logging is enabled */
#define DEBUG_LOG_GC

#define UINT8_COUNT (UINT8_MAX + 1)

#endif // CLOX_COMMON_H
