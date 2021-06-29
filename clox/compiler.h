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
 * @return The function object for the script
 */
FunctionObject* compile(const char* source);

#endif // CLOX_COMPILER_H
