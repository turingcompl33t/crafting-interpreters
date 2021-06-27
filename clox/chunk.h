/**
 * @file chunk.h
 */

#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

/**
 * Enumerates the operation codes supported by the clox VM.
 */
typedef enum {
  /** Load a constant from the constant pool */
  OP_CONSTANT,

  /** Load constant literals */
  OP_NIL,
  OP_TRUE,
  OP_FALSE,

  /** Explicit stack management */
  OP_POP,

  /** Variable definition */
  OP_GET_GLOBAL,
  OP_SET_GLOBAL,
  OP_DEFINE_GLOBAL,

  /** Equality and comparison operators */
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,

  /** Binary arithmetic operators */
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,

  /** Unary negation */
  OP_NOT,
  OP_NEGATE,

  /** Print statement */
  OP_PRINT,
  
  /** Return from a subroutine */
  OP_RETURN,
} OpCode;

/**
 * A chunk of bytecode and associated metadata.
 */
typedef struct {
  /** Current number of bytes used in the chunk */
  int count;
  /** Current total chunk capacity */
  int capacity;
  /** The bytecode stream */
  uint8_t* code;
  /** The line information for the bytecode stream */
  int* lines;
  /** The chunk constant pool */
  ValueArray constants;
} Chunk;

/**
 * Initialize a chunk.
 * @param chunk The chunk of interest
 */
void initChunk(Chunk* chunk);

/**
 * Deallocate all of the storage used by a chunk.
 * @param chunk The chunk of interst
 */
void freeChunk(Chunk* chunk);

/**
 * Write a byte at the end of the chunk.
 * @param chunk The chunk of interest
 * @param byte The byte to write
 * @param line The line number from source
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line);

/**
 * Add a constant value to the chunk constant pool.
 * @param chunk The chunk of interest
 * @param value The value to add to the pool
 * @return The index at which the value was added
 */
int addConstant(Chunk* chunk, Value value);

#endif // CLOX_CHUNK_H
