
/**
 * @file primitive.h
 * @author Devin Arena
 * @brief Helper functionality for primitives. Each primitive type is stored in
 * a dynamic array for each chunk.
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_DYNAMIC_ARRAY_H
#define PALLADIUM_DYNAMIC_ARRAY_H

#include "memory.h"

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

#define INIT_DYNAMIC_ARRAY(type, pointer) \
  do {                                    \
    (pointer)->data = NULL;               \
    (pointer)->count = 0;                 \
    (pointer)->capacity = 0;              \
  } while (false)

#define FREE_DYNAMIC_ARRAY(type, pointer)                   \
  do {                                                      \
    FREE_ARRAY(type, (pointer)->data, (pointer)->capacity); \
    (pointer)->data = NULL;                                 \
    (pointer)->count = 0;                                   \
    (pointer)->capacity = 0;                                \
  } while (false)

#define INSERT_DYNAMIC_ARRAY(type, pointer, value)                             \
  do {                                                                         \
    if (pointer->capacity < (pointer)->count + 1) {                            \
      int oldCapacity = (pointer)->capacity;                                   \
      (pointer)->capacity = GROW_CAPACITY(oldCapacity);                        \
      (pointer)->data =                                                        \
          GROW_ARRAY(type, (pointer)->data, oldCapacity, (pointer)->capacity); \
    }                                                                          \
    (pointer)->data[(pointer)->count++] = value;                               \
  } while (false)

#endif