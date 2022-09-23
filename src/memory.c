
/**
 * @file memory.c
 * @author Devin Arena
 * @brief Memory management for the VM implementation.
 * @since 6/22/2022
 **/

#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "table.h"

/**
 * @brief Reallocates memory for the specified pointer. Can be used to grow or
 * shrink. If newSize is 0, the memory is freed.
 *
 * @param pointer the pointer to resize
 * @param oldSize old size of the pointer
 * @param newSize new size of the pointer
 * @return void* the newly allocated pointer
 */
void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  // free memory if newSize is 0
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  // allocate new memory
  void* newPointer = realloc(pointer, newSize);

  // case where realloc fails
  if (newPointer == NULL) {
    printf("Could not reallocate memory\n");
    exit(74);
  }

  return newPointer;
}

void freeObject(Object* object) {
  switch (object->type) {
    case ObjectString: {
      PdString* str = (PdString*)object;
      FREE_ARRAY(char, str->chars, str->length + 1);
      FREE(ObjectString, object);
      break;
    }
    case ObjectFunction: {
      PdFunction* func = (PdFunction*)object;
      freeChunk(&func->chunk);
      FREE_DYNAMIC_ARRAY(Value, func->locals);
      FREE(ObjectFunction, object);
      break;
    }
    case ObjectBuiltin: {
      PdBuiltin* func = (PdBuiltin*)object;
      FREE_DYNAMIC_ARRAY(Value, func->argt);
      FREE(ObjectBuiltin, object);
      break;
    }
    case ObjectStructTemplate: {
      PdStructTemplate* pstruct = (PdStructTemplate*)object;
      freeTable(&pstruct->fieldTypes);
      FREE(ObjectStruct, object);
      break;
    }
    case ObjectStruct: {
      PdStruct* pstruct = (PdStruct*)object;
      freeTable(&pstruct->fields);
      FREE(ObjectStruct, object);
      break;
    }
    case ObjectReference: {
      PdReference* ref = (PdReference*)object;
      FREE(ObjectReference, object);
      break;
    }
    case ObjectModule: {
      PdModule* module = (PdModule*)object;
      freeTable(&module->globals);
      FREE(ObjectModule, object);
      break;
    }
  }
}

/**
 * @brief Reads a file from the specified path, returning the contents as a
 * null-terminated string.
 *
 * @param path const char* The path to the file to read.
 * @return char* The contents of the file.
 */
char* readFile(const char* path) {
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