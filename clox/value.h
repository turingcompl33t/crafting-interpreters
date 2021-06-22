/**
 * @file value.h
 */

#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

typedef double Value;

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

#endif // CLOX_VALUE_H
