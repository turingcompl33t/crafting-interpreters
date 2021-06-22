/**
 * @file memory.h
 */

#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"

#define GROW_CAPACITY(capacity) \
  ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
  (type*)reallocate(pointer, sizeof(type) * (oldCount), \
    sizeof(type) * (newCount));

#define FREE_ARRAY(type, pointer, oldCount) \
  reallocate(pointer, sizeof(type) * (oldCount), 0)

/**
 * Centralized function for memory allocation and deallocation.
 * @param ptr Existing pointer (for realloc() and free())
 * @param oldSize The old allocation size
 * @param newSize The new allocation size
 * @return The memory block (if not free())
 */
void* reallocate(void* ptr, size_t oldSize, size_t newSize);

#endif // CLOX_MEMORY_H
