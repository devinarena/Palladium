
/**
 * @file primitive.h
 * @author Devin Arena
 * @brief Helper functionality for primitives. Each primitive type is stored in
 * a dynamic array for each chunk.
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_DYNAMIC_ARRAY_H
#define PALLADIUM_DYNAMIC_ARRAY_H

#include <stdlib.h>

typedef struct Object Object;

#define FREE_ARRAY(type, pointer, count) \
  reallocate(pointer, sizeof(type) * count, 0)

// macro for growing capacity of an array, 8 to start, then double each time
#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

// macro for gorwing the array itself, reallocates necessary memory for the new
// size
#define GROW_ARRAY(type, pointer, oldCount, newCount) \
  (type*)reallocate(pointer, sizeof(type) * oldCount, sizeof(type) * newCount)

#define FREE(type, object) reallocate(object, sizeof(type), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void freeObject(Object* obj);

/**
 * @brief Dynamic arrays are going to require a lot of macros, ideally I want a
 * system where I can just specify what type for each function so I can use one
 * function for each.
 */

#define DYNAMIC_ARRAY(type) \
  struct {                  \
    type* data;             \
    int count;              \
    int capacity;           \
  }

#define INIT_DYNAMIC_ARRAY(type, array) \
  do {                                  \
    (array).data = NULL;                \
    (array).count = 0;                  \
    (array).capacity = 0;               \
  } while (false)

#define FREE_DYNAMIC_ARRAY(type, array)               \
  do {                                                \
    FREE_ARRAY(type, (array).data, (array).capacity); \
    (array).data = NULL;                              \
    (array).count = 0;                                \
    (array).capacity = 0;                             \
  } while (false)

#define INSERT_DYNAMIC_ARRAY(type, array, value)                         \
  do {                                                                   \
    if ((array).capacity < (array).count + 1) {                          \
      int oldCapacity = (array).capacity;                                \
      (array).capacity = GROW_CAPACITY(oldCapacity);                     \
      (array).data =                                                     \
          GROW_ARRAY(type, (array).data, oldCapacity, (array).capacity); \
    }                                                                    \
    (array).data[(array).count++] = value;                               \
  } while (false)

#define INSERT_DYNAMIC_ARRAY_AT(type, array, index, value) \
  do {                                                     \
    if ((array).count <= (index)) {                        \
      INSERT_DYNAMIC_ARRAY(type, array, value);            \
    } else {                                               \
      (array).data[(index)] = (value);                     \
    }                                                      \
  } while (false)

#endif