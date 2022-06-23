
/**
 * @file chunk.c
 * @author Devin Arena
 * @brief Implementation for chunk.h
 * @since 6/22/2022
 **/

#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

/**
 * @brief Initializes a chunk by zeroing out its memory.
 *
 * @param chunk Chunk* The chunk to initialize.
 */
void initChunk(Chunk* chunk) {
  chunk->code = NULL;
  chunk->lines = NULL;
  chunk->count = 0;
  chunk->capacity = 0;
  INIT_DYNAMIC_ARRAY(int, &chunk->const_int);
}

/**
 * @brief Writes the specified byte into the chunks code array. If the array
 * is not large enough, it will be resized by CHUNK_GROWTH_FACTOR.
 *
 * @param chunk Chunk* The chunk to write to.
 * @param byte uint8_t The byte to write.
 * @param line uint32_t The line number of the instruction.
 */
void writeToChunk(Chunk* chunk, uint8_t byte, uint32_t line) {
  // chunk does not have capacity to write to, so resize it
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code =
        GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    chunk->lines =
        GROW_ARRAY(uint32_t, chunk->lines, oldCapacity, chunk->capacity);
  }

  chunk->lines[chunk->count] = line;
  chunk->code[chunk->count++] = byte;
}

/**
 * @brief Frees a chunk by freeing its code array and zeroing out its memory.
 *
 * @param chunk Chunk* the chunk to free.
 */
void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  FREE_ARRAY(uint32_t, chunk->lines, chunk->capacity);
  FREE_DYNAMIC_ARRAY(int, &chunk->const_int);
  initChunk(chunk);
}

/**
 * @brief Helper method for printing the specified content to standard output.
 *
 * @param chunk Chunk* the chunk to print to
 * @param index int the index of the constant
 * @param type PrimitiveType the type of the constant
 */
void printConstant(Chunk* chunk, int index, PrimitiveType type) {
  switch (type) {
    case PRIMITIVE_INTEGER:
      printf("%d", chunk->const_int.data[index]);
      break;
    default:
      printf("Unknown constant type\n");
      exit(1);
  }
}