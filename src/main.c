/**
 * @file main.c
 * @author Devin Arena
 * @brief Entrypoint for the interpreter.
 * @since 6/22/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chunk.h"
#include "vm.h"
#include "memory.h"

/**
 * @brief Takes a source path, reads the file, tokenizes it, generates VM
 * bytecode, and interprets it.
 *
 * @param path const char* the path to the source file
 */
static void runFile(int argc, const char* argv[]) {
  char* source = readFile(argv[1]);

  interpret(source, argc, argv);

  free(source);
}

int main(int argc, const char* argv[]) {
  if (argc >= 2) {
    runFile(argc, argv);
  } else {
    fprintf(stderr, "Usage: palladium [script]\n");
    exit(64);
  }

  return 0;
}