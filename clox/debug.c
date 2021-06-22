/**
 * @file debug.c
 */

#include <stdio.h>

#include "debug.h"
#include "value.h"

// ----------------------------------------------------------------------------
// Local
// ----------------------------------------------------------------------------

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  // Grab the offset into the constant pool
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  // Print the value at the specified offset
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}

static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

// ----------------------------------------------------------------------------
// Exported
// ----------------------------------------------------------------------------

void disassembleChunk(Chunk* chunk, const char* name) {
  printf("=== %s ===\n", name);
  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

int disassembleInstruction(Chunk* chunk, int offset) {
  // Print the offset in the bytecode stream
  printf("%04d ", offset);

  // Print the source line number
  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  // Print the opcode information
  uint8_t instr = chunk->code[offset];
  switch (instr) {
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      printf("Unknown opcode %d\n", instr);
      return offset + 1;
  }
}