/**
 * @file object.h
 */

#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJECT_TYPE(value) (AS_OBJECT(value)->type)

#define IS_CLASS(value)    isObjectType(value, OBJ_CLASS)
#define IS_CLOSURE(value)  isObjectType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value) isObjectType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value) isObjectType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)   isObjectType(value, OBJ_NATIVE)
#define IS_STRING(value)   isObjectType(value, OBJ_STRING)

#define AS_CLASS(value)    ((ClassObject*)AS_OBJECT(value))
#define AS_CLOSURE(value)  ((ClosureObject*)AS_OBJECT(value))
#define AS_FUNCTION(value) ((FunctionObject*)AS_OBJECT(value))
#define AS_INSTANCE(value) ((InstanceObject*)AS_OBJECT(value))
#define AS_NATIVE(value)   (((NativeFnObject*)AS_OBJECT(value))->function)
#define AS_STRING(value)   ((StringObject*)AS_OBJECT(value))
#define AS_CSTRING(value)  (((StringObject*)AS_OBJECT(value))->data)

/**
 * ObjectType enumerates the types of Lox objects.
 */
typedef enum {
  OBJ_CLASS,
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_INSTANCE,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_UPVALUE
} ObjectType;

/**
 * The Object type defines the common "header" for all
 * Lox values that are of the object kind.
 */
struct Object {
  /** The type identifier for the object */
  ObjectType type;
  /** Denotes whether the object is marked during GC */
  bool isMarked;
  /** The next entry in the intrusive list of objects */
  struct Object* next;
};

/**
 * Print the object represented by `value`.
 * @param value The object to print
 */
void printObject(Value value);

/**
 * Get the string representation of an object type.
 * @param type The object type
 * @return A non-owning string representation of object type
 */
char* objectTypeString(ObjectType type);

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
 * The UpvalueObject type represents a Lox upvalue.
 */
typedef struct UpvalueObject {
  /** The common object header */
  Object object;
  /** The upvalue location */
  Value* location;
  /** The value itself, when upvalue is closed */
  Value closed;
  /** The next upvalue in the intrusive list of upvalues */
  struct UpvalueObject* next;
} UpvalueObject;

/**
 * Construct a new UpvalueObject.
 * @param slot The pointer to the referenced value
 * @return The upvalue object
 */
UpvalueObject* newUpvalue(Value* slot);

/**
 * The FunctionObject type represents a Lox function.
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The function's arity */
  int arity;
  /** The number of upvalues in the function */
  int upvalueCount;
  /** The function's bytecode chunk */
  Chunk chunk;
  /** The name of the function */
  StringObject* name;
} FunctionObject;

FunctionObject* newFunction();

/**
 * The ClosureObject type represents a Lox closure.
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The prototype for the closure */
  FunctionObject* function;
  /** The array of upvalues */
  UpvalueObject** upvalues;
  /** The number of upvalues in the array */
  int upvalueCount;
} ClosureObject;

/**
 * Construct a new closure object.
 * @param function The closure prototype
 * @return The closure object
 */
ClosureObject* newClosure(FunctionObject* function);

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
 * The ClassObject type represents a Lox class object.
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The name of the class */
  StringObject* name;
} ClassObject;

/**
 * Construct a new class object.
 * @param name The class name
 * @return The class object
 */
ClassObject* newClass(StringObject* name);

/**
 * The InstanceObject type represents a Lox class instance
 */
typedef struct {
  /** The common object header */
  Object object;
  /** The associated class object */
  ClassObject* klass;
  /** The fields of the instance */
  Table fields;
} InstanceObject;

/**
 * Construct a new instance object.
 * @param klass The associated class object
 * @return The instance object
 */
InstanceObject* newInstance(ClassObject* klass);

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
