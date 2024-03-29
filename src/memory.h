
/**
 * @file memory.h
 * @author Devin Arena
 * @brief Handles memory management (allocating, garbage collection, etc.) for
 *Palladium
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_MEMORY_H
#define PALLADIUM_MEMORY_H

#include "commons.h"
#include "object.h"

// macro for allocating memory of type and count
#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))

// macro for growing capacity of an array, 8 to start, then double each time
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

// macro for gorwing the array itself, reallocates necessary memory for the new
// size
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
  (type*)reallocate(pointer, sizeof(type) * oldCount, sizeof(type) * newCount)

#define FREE(type, object) reallocate(object, sizeof(type), 0)

// macro for freeing an array, calls reallocate with a size of 0 freeing the
// pointer
#define FREE_ARRAY(type, pointer, count) \
  reallocate(pointer, sizeof(type) * count, 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void freeObject(Object* obj);
char* readFile(const char* path);

#endif