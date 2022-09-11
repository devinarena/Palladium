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

/**
 * @brief Reads a file from the specified path, returning the contents as a
 * null-terminated string.
 *
 * @param path const char* The path to the file to read.
 * @return char* The contents of the file.
 */
static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");
  // file may not exist or be readable
  if (!file) {
    printf("Could not open file '%s'\n", path);
    exit(74);
  }

  // get file size and go back to beginning
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // allocate memory for file contents
  char* buffer = (char*)malloc(fileSize + 1);
  // may not have enough memory to read whole file
  if (buffer == NULL) {
    fprintf(stderr, "Not enough memory to read '%s'\n", path);
    exit(74);
  }

  // read file contents into buffer
  size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);

  // if bytes read is not equal to file size, something went wrong
  if (bytesRead < fileSize) {
    fprintf(stderr, "Could not read file '%s'\n", path);
    exit(74);
  }

  // null terminate buffer
  buffer[bytesRead] = '\0';

  // close file
  fclose(file);

  return buffer;
}

/**
 * @brief Takes a source path, reads the file, tokenizes it, generates VM
 * bytecode, and interprets it.
 *
 * @param path const char* the path to the source file
 */
static void runFile(int argc, const char* argv[]) {
  char* source = readFile(argv[1]);

  interpret(source);

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