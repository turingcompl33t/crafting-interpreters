/**
 * @file debug.h
 */

#ifndef CLOX_DEBUG_H
#define CLOX_DEBUG_H

#include "chunk.h"

/**
 * Disassemble a chunk.
 * @param chunk The chunk of interest
 * @param name An identifier for the chunk
 */
void disassembleChunk(Chunk* chunk, const char* name);

/**
 * Disassemble a single instruction within the chunk.
 * @param chunk The chunk in which the instruction appears
 * @param offset The offset in the chunk at which to begin disassembly
 * @return The updated offset
 */
int disassembleInstruction(Chunk* chunk, int offset);

#endif // CLOX_DEBUG_H
