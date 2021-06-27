/**
 * @file debug.c
 */

#include <stdio.h>

#include "debug.h"
#include "value.h"

// ----------------------------------------------------------------------------
// Local
// ----------------------------------------------------------------------------

/**
 * Print a constant instruction to standard output.
 * @param name The instruction name
 * @param chunk The instruction's chunk
 * @param offset The offset at which the instruction appears in the chunk
 * @return The updated offset
 */
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  // Grab the offset into the constant pool
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  // Print the value at the specified offset
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 2;
}

/**
 * Print a simple instruction to standard output.
 * @param name The instruction name
 * @param offset The offset at which the instruction appears in the chunk
 * @return The updated offset
 */
static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

/**
 * Print a byte instruction to standard output.
 * @param name The instruction name
 * @param chunk The instruction's chunk
 * @param offset The offset at which the instruction appears in the chunk
 * @return The updated offset
 */
static int byteInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
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
    case OP_NIL:
      return simpleInstruction("OP_NIL", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
      return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
      return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
      return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    default:
      printf("Unknown opcode %d\n", instr);
      return offset + 1;
  }
}