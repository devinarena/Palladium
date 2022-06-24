
/**
 * @file main.c
 * @author Devin Arena
 * @brief Entrypoint for the interpreter.
 * @since 5/19/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"

/**
 * @brief Reads a file and returns a source string.
 *
 * @param path const char* the path to the file.
 * @return char* the source string.
 */
static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
  if (!file) {
    printf("Could not open file '%s'\n", path);
    exit(74);
  }

  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  char* buffer = (char*)malloc(fileSize + 1);
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read '%s'\n", path);
    exit(74);
  }

  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file '%s'\n", path);
    exit(74);
  }

  buffer[bytesRead] = '\0';

  fclose(file);

  return buffer;
}

/**
 * @brief Runs a given source string through the scanner, compiler, and VM.
 *
 * @param path const char* the path to the file.
 */
static void runFile(const char* path) {
  char* source = readFile(path);
  InterpretResult result = interpret(source);
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

/**
 * @brief Entrypoint for the interpreter.
 *
 * @param argc command line argument count.
 * @param argv command line argument values.
 * @return int exit code.
 */
int main(int argc, const char* argv[]) {
  initVM();

  if (argc == 1) {
    // repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: palladium [script]\n");
    exit(64);
  }

  freeVM();
  return 0;
}