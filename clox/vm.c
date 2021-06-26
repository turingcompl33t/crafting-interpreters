/**
 * @file vm.c
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

/** The global virtual machine instance */
VM vm;

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

/**
 * Report a runtime error.
 * @param format The format string for the error message
 * @param variadic Variadic arguments to the format string
 */
static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = vm.ip - vm.chunk->code - 1;
  int line = vm.chunk->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);
  resetStack();
}

void initVM() {
  resetStack();
  vm.objects = NULL;
}

void freeVM() {
  freeObjects();
}

void push(Value value) {
  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

/**
 * Peek at a value on the runtime stack.
 * @param distance The distance to peek
 * @return The value at the requested stack location
 */
static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

/**
 * Determine if the given value is falsey.
 * @param value The value to check
 * @return `true` if the value is falsey, `false` otherwise
 */
static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
  StringObject* b = AS_STRING(pop());
  StringObject* a = AS_STRING(pop());

  int length = a->length + b->length;
  char* data = ALLOCATE(char, length + 1);
  memcpy(data, a->data, a->length);
  memcpy(data + a->length, b->data, b->length);
  data[length] = '\0';

  StringObject* result = takeString(data, length);
  push(OBJECT_VAL(result));
}

/**
 * The main virtual machine run loop.
 * @return The result of program interpretation
 */
static InterpretResult run() {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op)                                     \
  do {                                                               \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {                \
      runtimeError("Operands to binary operator must be numbers.");  \
      return INTERPRET_RUNTIME_ERROR;                                \
    }                                                                \
    double b = AS_NUMBER(pop());                                     \
    double a = AS_NUMBER(pop());                                     \
    push(valueType(a op b));                                         \
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
      case OP_NIL:   push(NIL_VAL); break;
      case OP_TRUE:  push(BOOL_VAL(true)); break;
      case OP_FALSE: push(BOOL_VAL(false)); break;

      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valueEquals(a, b)));
        break;
      }
      case OP_GREATER: BINARY_OP(BOOL_VAL, >); break;
      case OP_LESS:    BINARY_OP(BOOL_VAL, <); break;

      case OP_ADD: { 
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands for operator '+' not supported.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUBTRACT:  BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY:  BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:    BINARY_OP(NUMBER_VAL, /); break;
      
      case OP_NOT: {
        push(BOOL_VAL(isFalsey(pop())));
        break;
      }
      case OP_NEGATE: {
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand to '-' must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        push(NUMBER_VAL(-AS_NUMBER(pop()))); 
        break;
      }
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
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}
