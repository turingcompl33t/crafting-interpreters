/**
 * @file value.h
 */

#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

/**
 * ValueType enumerates all of the types of values
 * supported natively by the clox VM.
 */
typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER
} ValueType;

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
  } as;
} Value;

#define BOOL_VAL(value)    ((Value){VAL_BOOL, {.boolean=value}})
#define NIL_VAL            ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value)  ((Value){VAL_NUMBER, {.number = value}})

#define AS_BOOL(value)   ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

#define IS_BOOL(value)   ((value).type == VAL_BOOL)
#define IS_NIL(value)    ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)

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
