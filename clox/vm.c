/**
 * @file vm.c
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"

/** The global virtual machine instance */
VM vm;

/**
 * The clock() native function.
 * @param argCount The argument count (ignored)
 * @param args A pointer to function arguments (ignored)
 * @return UNIX timestamp as Lox value
 */
static Value clockNative(int argcCount, Value* args) {
  return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

/**
 * Reset the stack pointer.
 */
static void resetStack() {
  vm.stackTop = vm.stack;
  vm.frameCount = 0;
  vm.openUpvalues = NULL;
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

  for (int i = vm.frameCount - 1; i >= 0; i--) {
    CallFrame* frame = &vm.frames[i];
    FunctionObject* function = frame->closure->function;
    size_t instruction = frame->ip - function->chunk.code - 1;
    fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);
    if (function->name == NULL) {
      fprintf(stderr, "script\n");
    } else {
      fprintf(stderr, "%s()\n", function->name->data);
    }
  }
  
  resetStack();
}

/**
 * Helper function to define a new native function.
 * @param name The name of the native function
 * @param function The function pointer itself
 */
static void defineNative(const char* name, NativeFn function) {
  push(OBJECT_VAL(copyString(name, (int)strlen(name))));
  push(OBJECT_VAL(newNativeFn(function)));
  putTable(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
  pop();
  pop();
}

void initVM() {
  resetStack();
  
  vm.objects = NULL;

  vm.bytesAllocated = 0;
  vm.nextGC = GC_INITIAL_HEAP_CAPACITY;

  vm.grayCount = 0;
  vm.grayCapacity = 0;
  vm.grayStack = NULL;

  initTable(&vm.globals);
  initTable(&vm.strings);

  // Define the clock() native function
  defineNative("clock", clockNative);
}

void freeVM() {
  freeTable(&vm.globals);
  freeTable(&vm.strings);
  freeObjects();
}

size_t objectCountVM() {
  size_t count = 0;
  for (Object* object = vm.objects; object != NULL; object = object->next) {
    ++count;
  }
  return count;
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
  StringObject* b = AS_STRING(peek(0));
  StringObject* a = AS_STRING(peek(1));

  int length = a->length + b->length;
  char* data = ALLOCATE(char, length + 1);
  memcpy(data, a->data, a->length);
  memcpy(data + a->length, b->data, b->length);
  data[length] = '\0';

  StringObject* result = takeString(data, length);
  pop();
  pop();
  push(OBJECT_VAL(result));
}

/**
 * Call a Lox function object.
 * @param closure The closure to call
 * @param argCount The number of arguments to the call
 * @return `true` if the call is successful, `false` otherwise
 */
static bool call(ClosureObject* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.",
      closure->function->arity, argCount);
    return false;
  }

  if (vm.frameCount == FRAMES_MAX) {
    runtimeError("Stack overflow.");
    return false;
  }

  CallFrame* frame = &vm.frames[vm.frameCount++];
  frame->closure = closure;
  frame->ip = closure->function->chunk.code;
  frame->slots = vm.stackTop - argCount - 1;
  return true;
}

/**
 * Call the provided Lox value.
 * @param callee The value to call
 * @param argCount The number of arguments to the call
 * @return `true` if the call is successful, `false` otherwise
 */
static bool callValue(Value callee, int argCount) {
  if (IS_OBJECT(callee)) {
    switch (OBJECT_TYPE(callee)) {
      case OBJ_CLOSURE:
        return call(AS_CLOSURE(callee), argCount);
      case OBJ_NATIVE: {
        NativeFn native = AS_NATIVE(callee);
        Value result = native(argCount, vm.stackTop - argCount);
        vm.stackTop -= argCount + 1;
        push(result);
        return true;
      }
      default:
        break; // Non-callable object
    }
  }
  runtimeError("Invalid call target.");
  return false;
}

/**
 * Create a new upvalue for a captured local variable.
 * @param local The local slot
 * @return The captured upvalue
 */
static UpvalueObject* captureUpvalue(Value* local) {
  // Search for an existing upvalue at the same location
  UpvalueObject* prevUpvalue = NULL;
  UpvalueObject* upvalue = vm.openUpvalues;
  while (upvalue != NULL && upvalue->location > local) {
    prevUpvalue = upvalue;
    upvalue = upvalue->next;
  }

  if (upvalue != NULL && upvalue->location == local) {
    return upvalue;
  }

  // The upvalue does not already exist, so create it
  UpvalueObject* createdUpvalue = newUpvalue(local);
  
  // Splice the new upvalue into the intrusive list
  createdUpvalue->next = upvalue;
  if (prevUpvalue == NULL) {
    vm.openUpvalues = createdUpvalue;
  } else {
    prevUpvalue->next = createdUpvalue;
  }

  return createdUpvalue;
}

/**
 * Close all upvalues that appear above `last` on the stack.
 * @param last The last upvalue to close
 */
static void closeUpvalues(Value* last) {
  while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
    UpvalueObject* upvalue = vm.openUpvalues;
    upvalue->closed = *upvalue->location;
    upvalue->location = &upvalue->closed;
    vm.openUpvalues = upvalue->next;
  }
}

/**
 * The main virtual machine run loop.
 * @return The result of program interpretation
 */
static InterpretResult run() {
  // Grab a pointer to the current call frame
  CallFrame* frame = &vm.frames[vm.frameCount - 1];

#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() \
  (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() \
  (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
      disassembleInstruction(&frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
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

      case OP_POP: pop(); break;

      case OP_GET_LOCAL: {
        // Push the local variable onto the top of the runtime stack
        uint8_t slot = READ_BYTE();
        push(frame->slots[slot]);
        break;
      }
      case OP_SET_LOCAL: {
        // Read the local slot from the bytecode and set
        // the value of the local to the value that is
        // currently located at the top of the stack
        uint8_t slot = READ_BYTE();
        frame->slots[slot] = peek(0);
        break;
      }

      case OP_GET_GLOBAL: {
        // Load the value for a global variable
        StringObject* name = READ_STRING();
        Value value;
        if (!getTable(&vm.globals, name, &value)) {
          runtimeError("Undefined variable '%s'.", name->data);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        break;
      }
      case OP_SET_GLOBAL: {
        StringObject* name = READ_STRING();
        // putTable() returns `true` if a new entry is added;
        // in the case of assignment, this is a runtime error
        if (putTable(&vm.globals, name, peek(0))) {
          delTable(&vm.globals, name);
          runtimeError("Undefined variable '%s'.", name->data);
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }
      case OP_DEFINE_GLOBAL: {
        // Get the name of the variable from the constant table
        StringObject* name = READ_STRING();
        // Add the variable to the globals table with the name
        // as the key and the value as the value
        putTable(&vm.globals, name, peek(0));
        pop();
        break;
      }

      case OP_GET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        push(*frame->closure->upvalues[slot]->location);
        break;
      }
      case OP_SET_UPVALUE: {
        uint8_t slot = READ_BYTE();
        *frame->closure->upvalues[slot]->location = peek(0);
        break;
      }

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

      case OP_PRINT: {
        printValue(pop());
        printf("\n");
        break;
      }

      // Unconditional branch
      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        break;
      }
      // Conditional branch
      case OP_JUMP_IF_FALSE: {
        // Read the jump target from the bytecode stream
        uint16_t offset = READ_SHORT();
        // Evaluate the condition and update IP accordingly
        if (isFalsey(peek(0))) frame->ip += offset;
        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE();
        if (!callValue(peek(argCount), argCount)) {
          return INTERPRET_RUNTIME_ERROR;
        }
        // Update the frame to the called function
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }

      case OP_CLOSURE: {
        // Load the closure prototype from the constant pool
        FunctionObject* function = AS_FUNCTION(READ_CONSTANT());
        // Construct a new closure from this prototype
        ClosureObject* closure = newClosure(function);
        push(OBJECT_VAL(closure));
        
        // The magic moment where a closure comes to life
        for (int i = 0; i < closure->upvalueCount; ++i) {
          uint8_t isLocal = READ_BYTE();
          uint8_t index = READ_BYTE();
          if (isLocal) {
            closure->upvalues[i] = captureUpvalue(frame->slots + index);
          } else {
            closure->upvalues[i] = frame->closure->upvalues[index];
          }
        }
        break;
      }

      case OP_CLOSE_UPVALUE: {
        closeUpvalues(vm.stackTop - 1);
        pop();
        break;
      }

      case OP_RETURN: {
        Value result = pop();
        closeUpvalues(frame->slots);
        vm.frameCount--;
        if (vm.frameCount == 0) {
          pop();
          return INTERPRET_OK;
        }
        vm.stackTop = frame->slots;
        push(result);
        frame = &vm.frames[vm.frameCount - 1];
        break;
      }
    }
  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source) {
  // Compile the source program
  FunctionObject* function = compile(source);
  if (function == NULL) return INTERPRET_COMPILE_ERROR;

  // Prepare the interpreter to run the top-level function
  push(OBJECT_VAL(function));
  ClosureObject* closure = newClosure(function);
  pop();
  push(OBJECT_VAL(closure));
  call(closure, 0);

  return run();
}
