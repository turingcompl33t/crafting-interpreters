/**
 * @file object.c
 */

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJECT(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

/**
 * Allocate an object of the given size on the heap.
 * @param size The size of the object to allocate
 * @param type The object type
 * @return The allocated object
 */
static Object* allocateObject(size_t size, ObjectType type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;
  // Add the object to the linked list of objects in VM
  object->next = vm.objects;
  vm.objects = object;
  return object;
}

/**
 * Print a string representation of the function object to standard output.
 * @param function The function to print
 */
static void printFunction(FunctionObject* function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<fn %s>", function->name->data);
}

void printObject(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJ_FUNCTION:
      printFunction(AS_FUNCTION(value));
      break;
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}

/**
 * Create a new string object on the heap and initialize its fields.
 * @param data The string data
 * @param length The length of the string
 * @param hash The string hash value
 * @return The allocated string object
 */
static StringObject* allocateString(char* data, int length, uint32_t hash) {
  StringObject* string = ALLOCATE_OBJECT(StringObject, OBJ_STRING);
  string->length = length;
  string->data = data;
  string->hash = hash;
  // Automatically intern the string in the VM string table
  putTable(&vm.strings, string, NIL_VAL);
  return string;
}

/**
 * Compute the FNV-1a hash of a string.
 * @param data The string data
 * @param length The length of the string
 * @return The hash value
 */
static uint32_t hashString(const char* data, int length) {
  uint32_t hash = 2166136261U;
  for (int i = 0; i < length; ++i) {
    hash ^= (uint8_t)data[i];
    hash *= 16777619;
  }
  return hash;
}

StringObject* takeString(char* data, int length) {
  uint32_t hash = hashString(data, length);
  
  // Search for the string in the string table;
  // release the newly-allocated string if found
  // and return a reference to the existing string 
  StringObject* interned = findStringTable(&vm.strings, data, length, hash);
  if (interned != NULL) {
    FREE_ARRAY(char, data, length + 1);
    return interned;
  }
  return allocateString(data, length, hash);
}

StringObject* copyString(const char* data, int length) {
  uint32_t hash = hashString(data, length);
  
  // Search for the string in the string table;
  // elide the allocation if the string is present
  StringObject* interned = findStringTable(&vm.strings, data, length, hash);
  if (interned != NULL) return interned;

  // Get some memory on the heap for the string data
  char* heapChars = ALLOCATE(char, length + 1);
  
  // Copy the data into the heap buffer and terminate the string
  memcpy(heapChars, data, length);
  heapChars[length] = '\0';
  
  // Allocate the associated string object and initialize
  // it to refer to the heap buffer with the copied data
  return allocateString(heapChars, length, hash);
}

FunctionObject* newFunction() {
  FunctionObject* function = ALLOCATE_OBJECT(FunctionObject, OBJ_FUNCTION);
  function->arity = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}