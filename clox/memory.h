/**
 * @file memory.h
 */

#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count) \
  (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

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

/**
 * Release the memory for ALL allocated Lox objects.
 */
void freeObjects();

#endif // CLOX_MEMORY_H
