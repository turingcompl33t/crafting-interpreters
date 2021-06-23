/**
 * @file main.c
 */

#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, char* argv[]) {
  initVM();

  Chunk c;
  initChunk(&c);
  int constant = addConstant(&c, 1.2);
  writeChunk(&c, OP_CONSTANT, 123);
  writeChunk(&c, constant, 123);

  constant = addConstant(&c, 3.4);
  writeChunk(&c, OP_CONSTANT, 123);
  writeChunk(&c, constant, 123);

  writeChunk(&c, OP_ADD, 123);
  
  constant = addConstant(&c, 5.6);
  writeChunk(&c, OP_CONSTANT, 123);
  writeChunk(&c, constant, 123);
  
  writeChunk(&c, OP_DIVIDE, 123);
  writeChunk(&c, OP_NEGATE, 123);

  writeChunk(&c, OP_RETURN, 123);

  interpret(&c);

  freeChunk(&c);
  freeVM();

  return EXIT_SUCCESS;
}
