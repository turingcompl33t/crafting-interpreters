/**
 * @file memory.c 
 */

#include <stdlib.h>

#include "memory.h"
#include "vm.h"

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

/**
 * Release the memory used by a single Lox object.
 * @param object A pointer to the object
 */
static void freeObject(Object* object) {
  switch (object->type) {
    case OBJ_CLOSURE: {
      ClosureObject* closure = (ClosureObject*)object;
      // Free the array of upvalues for the closure
      // NOTE: The closure does not own the upvalues themselves,
      // but it DOES own the array of pointers to these upvalues
      FREE_ARRAY(UpvalueObject*, closure->upvalues, closure->upvalueCount);
      // NOTE: The closure does not also free
      // its prototype because it may be shared
      // by multiple active closures
      FREE(ClosureObject, object);
      break;
    }
    case OBJ_FUNCTION: {
      FunctionObject* function = (FunctionObject*)object;
      freeChunk(&function->chunk);
      FREE(FunctionObject, object);
      break;
    }
    case OBJ_NATIVE: {
      FREE(NativeFnObject, object);
      break;
    }
    case OBJ_STRING: {
      StringObject* string = (StringObject*)object;
      FREE_ARRAY(char, string->data, string->length + 1);
      FREE(StringObject, object);
      break;
    }
    case OBJ_UPVALUE: {
      FREE(UpvalueObject, object);
      break;
    }
  }
}

void freeObjects() {
  Object* object = vm.objects;
  while (object != NULL) {
    Object* next = object->next;
    freeObject(object);
    object = next;
  }
}
