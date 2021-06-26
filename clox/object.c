/**
 * @file object.c
 */

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
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
 * Create a new string object on the heap and initialize its fields.
 * @param data The string data
 * @param length The length of the string
 * @return The allocated string object
 */
static StringObject* allocateString(char* data, int length) {
  StringObject* string = ALLOCATE_OBJECT(StringObject, OBJ_STRING);
  string->length = length;
  string->data = data;
  return string;
}

StringObject* takeString(char* data, int length) {
  return allocateString(data, length);
}

StringObject* copyString(const char* data, int length) {
  // Get some memory on the heap for the string data
  char* heapChars = ALLOCATE(char, length + 1);
  
  // Copy the data into the heap buffer and terminate the string
  memcpy(heapChars, data, length);
  heapChars[length] = '\0';
  
  // Allocate the associated string object and initialize
  // it to refer to the heap buffer with the copied data
  return allocateString(heapChars, length);
}

void printObject(Value value) {
  switch (OBJECT_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}