/**
 * @file vm.c
 */

#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "vm.h"

// The global virtual machine instance
static VM vm;

/**
 * Reset the stack pointer.
 */
static void resetStack() {
  vm.stackTop = vm.stack;
}

/**
 * Dump the contents of the runtime stack to standard output.
 */
static void dumpStack() {
  printf("          ");
  for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
    printf("[ ");
    printValue(*slot);
    printf(" ]");
  }
  printf("\n");
}

void initVM() {
  resetStack();
}

void freeVM() {

}

/**
 * The main virtual machine run loop.
 * @return The result of program interpretation
 */
static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op)  \
  do {                 \
    double b = pop();  \
    double a = pop();  \
    push(a op b);      \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
      dumpStack();
      disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t instr;
    switch (instr = READ_BYTE()) {
      case OP_CONSTANT: {
        push(READ_CONSTANT());
        break;
      }
      case OP_ADD: BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE: BINARY_OP(/); break;
      case OP_NEGATE: push(-pop()); break;
      case OP_RETURN: {
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
      }
    }
  }

#undef BINARY_OP
#undef READ_BYTE
#undef READ_CONSTANT
}

InterpretResult interpret(const char* source) {
  compile(source);
  run();
  return INTERPRET_OK;
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}