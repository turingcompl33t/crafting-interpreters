/**
 * @file value.h
 */

#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include <string.h>

#include "common.h"

/**
 * ValueType enumerates all of the types of values
 * supported natively by the clox VM.
 */
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER,
  VAL_OBJECT
} ValueType;

/** Forward declaration for the Object "base class" */
typedef struct Object Object;
/** Forward declaration for the StringObject type */
typedef struct StringObject StringObject;

#ifdef NAN_BOXING

/** Sign bit */
#define SIGN_BIT ((uint64_t)0x8000000000000000)
/** Quite NaN */
#define QNAN     ((uint64_t)0x7ffc000000000000)

#define TAG_NIL   1  // 01
#define TAG_FALSE 2  // 10
#define TAG_TRUE  3  // 11

typedef uint64_t Value;

#define BOOL_VAL(b)        ((b) ? TRUE_VAL : FALSE_VAL)
#define FALSE_VAL          ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define TRUE_VAL           ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define NIL_VAL            ((Value)(uint64_t)(QNAN | TAG_NIL))
#define NUMBER_VAL(number) numToValue(number)
#define OBJECT_VAL(object) \
  (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(object))

#define AS_BOOL(value)   ((value) == TRUE_VAL)
#define AS_NUMBER(value) valueToNumber(value)
#define AS_OBJECT(value) \
  ((Object*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

#define IS_BOOL(value)    (((value) | 1) == TRUE_VAL)
#define IS_NIL(value)     ((value) == NIL_VAL)
#define IS_NUMBER(value)  (((value) & QNAN) != QNAN)
#define IS_OBJECT(value) \
  (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

/**
 * Convert a double numeric value to an equivalent Value representation.
 * @param number The input number
 * @return The value
 */
static inline Value numToValue(double number) {
  Value value;
  memcpy(&value, &number, sizeof(double));
  return value;
}

/**
 * Convert a Value to an equivalent double numeric value.
 * @param value The input value
 * @return The number
 */
static inline double valueToNumber(Value value) {
  double number;
  memcpy(&number, &value, sizeof(Value));
  return number;
}

#else 
/**
 * The Value type abstracts over the notion of native
 * runtime values in the clox VM.
 */
typedef struct {
  /** The type of the value */
  ValueType type;
  /** The storage for the underlying value */
  union {
    bool boolean;
    double number;
    Object* object;
  } as;
} Value;

#define BOOL_VAL(value)    ((Value){VAL_BOOL,   {.boolean = value}})
#define NIL_VAL            ((Value){VAL_NIL,    {.number = 0}})
#define NUMBER_VAL(value)  ((Value){VAL_NUMBER, {.number = value}})
#define OBJECT_VAL(value)  ((Value){VAL_OBJECT, {.object = (Object*)value}})

#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJECT(value) ((value).as.object)

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)
#define IS_OBJECT(value)  ((value).type == VAL_OBJECT)

#endif // NAN_BOXING

typedef struct {
  /** The current number of values in the array */
  int count;
  /** The current total capacity of the array */
  int capacity;
  /** The dynamic array of values */
  Value* values;
} ValueArray;

/**
 * Initialize a value array.
 * @param array The array
 */
void initValueArray(ValueArray* array);

/**
 * Deallocate all of the storage used by the value array.
 * @param array The array
 */
void freeValueArray(ValueArray* array);

/**
 * Write a new value at the end of the value array.
 * @param array The array
 * @param value The value to write
 */
void writeValueArray(ValueArray* array, Value value);

/**
 * Print a value to standard output.
 * @param value The value
 */
void printValue(Value value);

/**
 * Determine if two values are equal.
 * @param a Input value
 * @param b Input value
 * @return `true` if `a` and `b` are equal, `false` otherwise
 */
bool valueEquals(Value a, Value b);

#endif // CLOX_VALUE_H
