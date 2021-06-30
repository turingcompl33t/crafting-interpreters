/**
 * @file object.h
 */

#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "chunk.h"
#include "value.h"

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define IS_FUNCTION(value) isObjectType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)   isObjectType(value, OBJ_NATIVE)
#define IS_STRING(value)   isObjectType(value, OBJ_STRING)

#define AS_FUNCTION(value) ((FunctionObject*)AS_OBJECT(value))
#define AS_NATIVE(value)   (((NativeFnObject*)AS_OBJECT(value))->function)
#define AS_STRING(value)   ((StringObject*)AS_OBJECT(value))
#define AS_CSTRING(value)  (((StringObject*)AS_OBJECT(value))->data)

/**
 * ObjectType enumerates the types of Lox objects.
 */
typedef enum {
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
} ObjectType;

/**
 * The Object type defines the common "header" for all
 * Lox values that are of the object kind.
 */
struct Object {
  /** The type identifier for the object */
  ObjectType type;
  /** The next entry in the intrusive list of objects */
  struct Object* next;
};

/**
 * Print the object represented by `value`.
 * @param value The object to print
 */
void printObject(Value value);

/**
 * The StringObject type implements the Lox string type.
 * 
 * Lox Strings are implemented with two layers of indirection:
 * 
 * - The Lox Value contains a pointer to the StringObject
 * - The StringObject contains the required Object header,
 *   the length of the string, and a pointer to the string's
 *   data which is allocated elsewhere on the heap
 */
struct StringObject {
  /** The common Object header */
  Object object;
  /** The number of characters in the string */
  int length;
  /** The string character array */
  char* data;
  /** The cached hash value for the string */
  uint32_t hash;
};

/**
 * Allocate a Lox string object and assume ownership of the provided data.
 * @param data The string data
 * @param length The length of the string
 * @return The allocated string object
 */
StringObject* takeString(char* data, int length);

/**
 * Allocate a Lox string object and copy the provided data to it.
 * @param data The string data
 * @param length The length of the string
 * @return The allocated string object
 */
StringObject* copyString(const char* data, int length);

/**
 * The FunctionObject type represents a Lox function.
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The function's arity */
  int arity;
  /** The function's bytecode chunk */
  Chunk chunk;
  /** The name of the function */
  StringObject* name;
} FunctionObject;

FunctionObject* newFunction();

/** The type signature for a native function type */
typedef Value (*NativeFn)(int argCount, Value* args);

/**
 * The NativeObject type represents a Lox native function object.
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The native function pointer */
  NativeFn function;
} NativeFnObject;

/**
 * Construct a new native function object.
 * @param function The native function pointer
 * @return The native function object
 */
NativeFnObject* newNativeFn(NativeFn function);

/**
 * Determine if the value is an object of the specified type.
 * @param value The Lox value
 * @param type The object type for which to query
 * @return `true` if the value is an object and matches the
 * specified object type, `false` otherwise
 */
static inline bool isObjectType(Value value, ObjectType type) {
  return IS_OBJECT(value) && AS_OBJECT(value)->type == type;
}

#endif // CLOX_OBJECT_H
