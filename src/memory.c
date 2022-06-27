
/**
 * @file memory.c
 * @author Devin Arena
 * @brief Memory management for the VM implementation.
 * @since 6/22/2022
 **/

#include <stdlib.h>
#include <stdio.h>

#include "memory.h"

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
  switch(object->type) {
    case ObjectString: {
      ObjString* str = (ObjString*)object;
      FREE_ARRAY(char, str->chars, str->length + 1);
      FREE(ObjectString, object);
      break;
    }
  }
}