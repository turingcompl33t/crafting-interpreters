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
 * Log object allocation event.
 * @param object The object that is allocated
 * @param size The size of the object
 */
static void logAllocateObject(Object* object, size_t size) {
  printf("%p allocate %zu for %s\n",
    (void*)object, size, objectTypeString(object->type));
}

/**
 * Allocate an object of the given size on the heap.
 * @param size The size of the object to allocate
 * @param type The object type
 * @return The allocated object
 */
static Object* allocateObject(size_t size, ObjectType type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;
  object->isMarked = false;
  // Add the object to the linked list of objects in VM
  object->next = vm.objects;
  vm.objects = object;

#ifdef DEBUG_LOG_GC
  logAllocateObject(object, size);
#endif

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
    case OBJ_CLASS:
      printf("%s", AS_CLASS(value)->name->data);
      break;
    case OBJ_CLOSURE:
      printFunction(AS_CLOSURE(value)->function);
      break;
    case OBJ_FUNCTION:
      printFunction(AS_FUNCTION(value));
      break;
    case OBJ_INSTANCE:
      printf("%s instance", AS_INSTANCE(value)->klass->name->data);
      break;
    case OBJ_NATIVE:
      printf("<native function>\n");
      break;
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
    case OBJ_UPVALUE:
      printf("upvalue");
      break;
  }
}

char* objectTypeString(ObjectType type) {
  switch (type) {
    case OBJ_CLOSURE:
      return "OBJ_CLOSURE";
    case OBJ_FUNCTION:
      return "OBJ_FUNCTION";
    case OBJ_NATIVE:
      return "OBJ_NATIVE";
    case OBJ_STRING:
      return "OBJ_STRING";
    case OBJ_UPVALUE:
      return "OBJ_UPVALUE";
    default:
      return "UNKNOWN TYPE";
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

  // Automatically intern the string in the VM string table;
  // note that we need to push and pop the string object here
  // in order to ensure that a garbage collection that is 
  // triggered by the putTable() operation does not collect a
  // string that it not yet added as a root
  push(OBJECT_VAL(string));
  putTable(&vm.strings, string, NIL_VAL);
  pop();

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

UpvalueObject* newUpvalue(Value* slot) {
  UpvalueObject* upvalue = ALLOCATE_OBJECT(UpvalueObject, OBJ_UPVALUE);
  upvalue->location = slot;
  upvalue->closed = NIL_VAL;
  upvalue->next = NULL;
  return upvalue;
}

FunctionObject* newFunction() {
  FunctionObject* function = ALLOCATE_OBJECT(FunctionObject, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

ClosureObject* newClosure(FunctionObject* function) {
  UpvalueObject** upvalues = ALLOCATE(UpvalueObject*, function->upvalueCount);
  for (int i = 0; i < function->upvalueCount; ++i) {
    upvalues[i] = NULL;
  }

  ClosureObject* closure = ALLOCATE_OBJECT(ClosureObject, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
  return closure;
}

NativeFnObject* newNativeFn(NativeFn function) {
  NativeFnObject* native = ALLOCATE_OBJECT(NativeFnObject, OBJ_NATIVE);
  native->function = function;
  return native;
}

ClassObject* newClass(StringObject* name) {
  ClassObject* klass = ALLOCATE_OBJECT(ClassObject, OBJ_CLASS);
  klass->name = name;
  return klass;
}

InstanceObject* newInstance(ClassObject* klass) {
  InstanceObject* instance = ALLOCATE_OBJECT(InstanceObject, OBJ_INSTANCE);
  instance->klass = klass;
  initTable(&instance->fields);
  return instance;
}