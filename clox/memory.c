/**
 * @file memory.c 
 */

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "memory.h"
#include "vm.h"

#ifdef DEBUG_LOG_GC
#include "debug.h"

/**
 * Log object deallocation event.
 * @param object The object that is freed
 */
static void logFreeObject(Object* object) {
  printf("%p free type %s", (void*)object, objectTypeString(object->type));
  switch (object->type) {
    case OBJ_STRING: {
      StringObject* string = (StringObject*)object;
      printf(" (%.*s)", string->length, string->data);
      break;
    }
    case OBJ_CLASS: {
      ClassObject* klass = (ClassObject*)object;
      printf(" (%.*s)", klass->name->length, klass->name->data);
      break;
    }
    case OBJ_INSTANCE: {
      InstanceObject* instance = (InstanceObject*)object;
      printf(" (%.*s instance)",
        instance->klass->name->length, instance->klass->name->data);
      break;
    }
    case OBJ_CLOSURE:
    case OBJ_FUNCTION:
    case OBJ_NATIVE:
    case OBJ_UPVALUE:
    case OBJ_BOUND_METHOD:
      break;
  }
  printf("\n");
} 
#endif // DEBUG_LOG_GC

/** The factor by which the heap is expanded or contracted on GC */
#define GC_HEAP_GROW_FACTOR 2

/** @return `true` if a garbage collection should be triggered. */
static bool shouldRunGC() {
#ifdef DEBUG_STRESS_GC
  // Trigger a GC whenever additional memory is requested
  return true;
#else    
  // Trigger a GC when the current threshold is hit
  return vm.bytesAllocated > vm.nextGC;
#endif
}

void* reallocate(void* ptr, size_t oldSize, size_t newSize) {
  vm.bytesAllocated += (newSize - oldSize);

  if (newSize > oldSize && shouldRunGC()) {
    collectGarbage();
  }

  if (newSize == 0) {
    free(ptr);
    return NULL;
  }

  // realloc() with NULL pointer behaves like malloc()
  void* result = realloc(ptr, newSize);
  if (result == NULL) exit(99);
  return result;
}

/**
 * Release the memory used by a single Lox object.
 * @param object A pointer to the object
 */
static void freeObject(Object* object) {
#ifdef DEBUG_LOG_GC
  logFreeObject(object);
#endif

  switch (object->type) {
    case OBJ_BOUND_METHOD:
      FREE(BoundMethodObject, object);
      break;
    case OBJ_CLASS: {
      ClassObject* klass = (ClassObject*)object;
      freeTable(&klass->methods);
      FREE(ClassObject, object);
      break;
    }
    case OBJ_CLOSURE: {
      ClosureObject* closure = (ClosureObject*)object;
      // Free the array of upvalues for the closure
      // NOTE: The closure does not own the upvalues themselves,
      // but it DOES own the array of pointers to these upvalues
      FREE_ARRAY(UpvalueObject*, closure->upvalues, closure->upvalueCount);
      // NOTE: The closure does not also free
      // its prototype because it may be shared
      // by multiple active closures
      FREE(ClosureObject, object);
      break;
    }
    case OBJ_FUNCTION: {
      FunctionObject* function = (FunctionObject*)object;
      freeChunk(&function->chunk);
      FREE(FunctionObject, object);
      break;
    }
    case OBJ_INSTANCE: {
      InstanceObject* instance = (InstanceObject*)object;
      freeTable(&instance->fields);
      FREE(InstanceObject, object);
      break;
    }
    case OBJ_NATIVE: {
      FREE(NativeFnObject, object);
      break;
    }
    case OBJ_STRING: {
      StringObject* string = (StringObject*)object;
      FREE_ARRAY(char, string->data, string->length + 1);
      FREE(StringObject, object);
      break;
    }
    case OBJ_UPVALUE: {
      FREE(UpvalueObject, object);
      break;
    }
  }
}

void freeObjects() {
  Object* object = vm.objects;
  while (object != NULL) {
    Object* next = object->next;
    freeObject(object);
    object = next;
  }

  // Free the unmanaged gray stack
  free(vm.grayStack);
}

void markValue(Value value) {
  if (IS_OBJECT(value)) markObject(AS_OBJECT(value));
}

void markObject(Object* object) {
  if (object == NULL) return;
  if (object->isMarked) return;

#ifdef DEBUG_LOG_GC
  printf("%p mark ", (void*)object);
  printValue(OBJECT_VAL(object));
  printf("\n");
#endif

  object->isMarked = true;

  if (vm.grayCapacity < vm.grayCount + 1) {
    vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
    vm.grayStack = (Object**) realloc(vm.grayStack, sizeof(Object*) * vm.grayCapacity);
    if (vm.grayStack == NULL) exit(99);
  }
  vm.grayStack[vm.grayCount++] = object;
}

/**
 * Mark all values in array `array` during GC.
 * @param array The array to mark
 */
static void markArray(ValueArray* array) {
  for (int i = 0; i < array->count; ++i) {
    markValue(array->values[i]);
  }
}

/**
 * Mark an object as `black` (all nested references marked) during GC.
 * @param object The object to blacken
 */
static void blackenObject(Object* object) {
#ifdef DEBUG_LOG_GC
  printf("%p blacken ", (void*)object);
  printValue(OBJECT_VAL(object));
  printf("\n");
#endif

  switch (object->type) {
    case OBJ_BOUND_METHOD: {
      BoundMethodObject* bound = (BoundMethodObject*)object;
      markValue(bound->receiver);
      markObject((Object*)bound->method);
      break;
    }
    case OBJ_CLASS: {
      ClassObject* klass = (ClassObject*)object;
      markObject((Object*)klass->name);
      markForGCTable(&klass->methods);
      break;
    }
    case OBJ_CLOSURE: {
      ClosureObject* closure = (ClosureObject*)object;
      markObject((Object*)closure->function);
      for (int i = 0; i < closure->upvalueCount; ++i) {
        markObject((Object*)closure->upvalues[i]);
      }
      break;
    }
    case OBJ_FUNCTION: {
      FunctionObject* function = (FunctionObject*)object;
      // Mark the string allocated for function name
      markObject((Object*)function->name);
      // Recurse into the function's constant pool
      markArray(&function->chunk.constants);
      break;
    }
    case OBJ_INSTANCE: {
      InstanceObject* instance = (InstanceObject*)object;
      markObject((Object*)instance->klass);
      markForGCTable(&instance->fields);
      break;
    }
    case OBJ_UPVALUE:
      // When an upvalue is closed, it contains a
      // reference to the closed over value that lives
      // on the heap, which we now mark for visitation
      markValue(((UpvalueObject*)object)->closed);
      break;
    case OBJ_NATIVE:
    case OBJ_STRING:
      break;
  }
}

/**
 * Mark all directly-reachable roots for garbage collection.
 */
static void markRoots() {
  // Mark the values that live on the stack
  for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }

  // TODO: The call stack is distinct from the VM runtime stack?
  // I need to revisit the high-level structure of the VM and
  // really understand why two distinct stacks are necessary...

  // Mark objects that live on the call frame stack
  for (int i = 0; i < vm.frameCount; ++i) {
    markObject((Object*)vm.frames[i].closure);
  }

  // Mark open upvalues
  for (UpvalueObject* upvalue = vm.openUpvalues; upvalue != NULL; upvalue = upvalue->next) {
    markObject((Object*)upvalue);
  }

  // Mark globals
  markForGCTable(&vm.globals);

  // Mark the roots accessible only to the compiler
  markCompilerRoots();
  markObject((Object*)vm.initString);
}

/**
 * Trace all marked roots for garbage collection.
 */
static void traceReferences() {
  while (vm.grayCount > 0) {
    Object* object = vm.grayStack[--vm.grayCount];
    blackenObject(object);
  }
}

/**
 * Sweep unreferenced objects during garbage collection.
 */
static void sweep() {
  Object* previous = NULL;
  Object* object = vm.objects;
  while (object != NULL) {
    if (object->isMarked) {
      object->isMarked = false;
      previous = object;
      object = object->next;
    } else {
      Object* unreached = object;
      object = object->next;
      if (previous != NULL) {
        previous->next = object;
      } else {
        vm.objects = object;
      }

      freeObject(unreached);
    }
  }
}

void collectGarbage() {
#ifdef DEBUG_LOG_GC
  printf("-- GC BEGIN (%zu managed objects)\n", objectCountVM());
  size_t before = vm.bytesAllocated;
#endif

markRoots();
traceReferences();
removeWeakRefsTable(&vm.strings);
sweep();

vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
  printf("-- GC END (%zu managed objects)\n", objectCountVM());
  printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
    before - vm.bytesAllocated, before, vm.bytesAllocated, vm.nextGC);
#endif
}
