/**
 * @file vm.h
 */

#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "object.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

/** The maximum number of active call frames */
#define FRAMES_MAX 64
/** The maximum size of the clox VM runtime stack */
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

/**
 * The CallFrame type represents a single in-flight function invocation.
 */
typedef struct {
  /** The function object associated with this call frame */
  ClosureObject* closure;
  /** The current instruction pointer for this frame */
  uint8_t* ip;
  /** A pointer into the VM's stack where the slots for this call begin */
  Value* slots;
} CallFrame;

/**
 * A VM instance encapsulates all of the state
 * necessary to interpret a chunk of bytecode.
 */
typedef struct {
  /** The implicit stack of call frames */
  CallFrame frames[FRAMES_MAX];
  /** The current number of active frames */
  int frameCount;
  /** The virtual machine runtime stack */
  Value stack[STACK_MAX];
  /** A pointer to the top of the stack (one past the final element)*/
  Value* stackTop;
  /** The head of the linked list of allocated objects */
  Object* objects;
  /** Global variables */
  Table globals;
  /** The string-interning table */
  Table strings;
  /** Instrusive linked list of open upvalues */
  UpvalueObject* openUpvalues;
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

/** Expose the global VM instance externally */
extern VM vm;

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
