/**
 * @file memory.c 
 */

#include <stdlib.h>

#include "memory.h"

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(ptr);
    return NULL;
  }

  // realloc() with NULL pointer behaves like malloc()
  void* result = realloc(ptr, newSize);
  if (result == NULL) exit(1);
  return result;
}
