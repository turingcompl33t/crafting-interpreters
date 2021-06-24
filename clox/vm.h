/**
 * @file vm.h
 */

#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"

/** The maximum size of the clox VM runtime stack */
#define STACK_MAX 256

/**
 * A VM instance encapsulates all of the state
 * necessary to interpret a chunk of bytecode.
 */
typedef struct {
  /** The chunk of bytecode that the VM interprets */
  Chunk* chunk;
  /** The virtual machine instruction pointer */
  uint8_t* ip;
  /** The virtual machine runtime stack */
  Value stack[STACK_MAX];
  /** A pointer to the top of the stack (one past the final element)*/
  Value* stackTop;
} VM;

/**
 * InterpretResult denotes the result of interpreting
 * a chunk of bytecode on the virtual machine.
 */
typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

/**
 * Initialize the virtual machine.
 */
void initVM();

/**
 * Deallocate all of the resources used by the virtual machine.
 */
void freeVM();

/**
 * Interpret a chunk of bytecode on the virtual machine.
 * @param source The Lox source to interpret
 * @return The result of interpretation
 */
InterpretResult interpret(const char* source);

/**
 * Push a value onto the runtime stack.
 * @param value The value
 */
void push(Value value);

/**
 * Pop a value off the runtime stack.
 * @return The value
 */
Value pop();

#endif // CLOX_VM_H
