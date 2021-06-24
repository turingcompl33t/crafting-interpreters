/**
 * @file main.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "error.h"
#include "vm.h"

#define MAX_LINE (1024)

/**
 * Run clox in a read-eval-print loop.
 */
static void repl() {
  char line[MAX_LINE];
  for (;;) {
    printf("> ");
    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }
    interpret(line);
  }
}

/**
 * Read the contents of a Lox source file to a buffer.=
 * @param path The file path
 * @return The file contents buffer
 */
static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file \"%s\".\n", path);
    exit(EX_IOERR);
  }
  
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*) malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to allocate\n");
    exit(EX_IOERR);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file \"%s\".\n", path);
    exit(EX_IOERR);
  }

  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

/**
 * Run the Lox source program at `path`.
 * @param path The file path
 */
static void runFile(const char* path) {
  char* source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR) exit(EX_DATAERR);
  if (result == INTERPRET_RUNTIME_ERROR) exit(EX_SOFTWARE);
}

int main(int argc, char* argv[]) {
  initVM();

  if (argc == 1) {
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: clox [path]\n");
    exit(EX_USAGE);
  }

  freeVM();
  return EXIT_SUCCESS;
}
