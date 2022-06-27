
/**
 * @file object.c
 * @author Devin Arena
 * @brief Object representation in Palladium.
 * @since 6/25/2022
 **/

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

/**
 * @brief Allocates an object onto the heap with the given size and type.
 *
 * @param size The size of the object to allocate.
 * @param type The type of the object to allocate.
 * @return Object* The pointer to the allocated object.
 */
static Object* allocateObject(size_t size, ObjectType type) {
  Object* object = (Object*)reallocate(NULL, 0, size);
  object->type = type;

  object->next = vm.heap;
  vm.heap = object;

#ifdef DEBUG_LOG_GC
  printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

  return object;
}

/**
 * @brief Allocates a new string onto the heap.
 *
 * @param chars The characters of the string to allocate.
 * @param length The length of the string to allocate.
 * @param hash The hash of the string to allocate.
 * @return ObjString* The pointer to the allocated string.
 */
static ObjString* allocateString(char* chars, int length, uint32_t hash) {
  ObjString* string = ALLOCATE_OBJ(ObjString, ObjectString);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  push(FROM_OBJECT(string));
  tableSet(&vm.strings, string, NULL_VAL);
  pop();
  return string;
}

/**
 * @brief Hashes a string using a basic hashing algorithm.
 *
 * @param key The string to hash.
 * @param length The length of the string to hash.
 * @return uint32_t The hash of the string.
 */
static uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }

  return hash;
}

/**
 * @brief Allocates a new string for the ObjString* given a c string.
 *
 * @param chars The c string to allocate.
 * @param length The length of the c string to allocate.
 * @return ObjString* The pointer to the allocated string.
 */
ObjString* newString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
}

/**
 * @brief Copies a c string into a new string and the VM's string table.
 *
 * @param chars The c string to copy.
 * @param length The length of the c string to copy.
 * @return ObjString* The pointer to the copied string.
 */
ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL)
    return interned;

  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash);
}

/**
 * @brief Outputs an object to standard output.
 *
 * @param value
 */
void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case ObjectString:
      printf("%s", TO_CSTRING(value));
      break;
    default:
      printf("%p", TO_OBJECT(value));
      break;
  }
}