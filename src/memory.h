#ifndef PALLADIUM_MEMORY_H
#define PALLADIUM_MEMORY_H

#include "common.h"
#include "value.h"
#include "object.h"

#define ALLOCATE(type, count) (type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

// 8 bytes of padding, wastes memory on very small chunks but better
// because it prevents requiring growing multiple times when small
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
  (type*)reallocate(pointer, sizeof(type) * oldCount, sizeof(type) * newCount)

#define FREE_ARRAY(type, pointer, oldCount) \
  reallocate(pointer, sizeof(type) * oldCount, 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void markValue(Value value);
void markObject(Obj* obj);
void collectGarbage();
void freeObjects();

#endif