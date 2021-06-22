/**
 * @file main.c
 */

#include <stdlib.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"

int main(int argc, char* argv[]) {
  Chunk c;
  initChunk(&c);
  int constant = addConstant(&c, 1.2);
  writeChunk(&c, OP_CONSTANT, 123);
  writeChunk(&c, constant, 123);
  writeChunk(&c, OP_RETURN, 123);
  disassembleChunk(&c, "test chunk");
  freeChunk(&c);


  return EXIT_SUCCESS;
}
