/**
 * @file compiler.h
 */

#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "object.h"
#include "vm.h"

/**
 * Compile the Lox source program.
 * @param source The Lox source program buffer
 * @param chunk The chunk to which the compiled bytecode is written
 * @return `true` if compilation succeeds, `false` otherwise
 */
bool compile(const char* source, Chunk* chunk);

#endif // CLOX_COMPILER_H
