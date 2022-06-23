
/**
 * @file chunk.c
 * @author Devin Arena
 * @brief Implementation for chunk.h
 * @since 6/22/2022
 **/

#include "chunk.h"
#include "memory.h"
#include "vm.h"


/**
 * @brief Initializes a chunk by zeroing out its memory.
 *
 * @param chunk Chunk* The chunk to initialize.
 */
void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

/**
 * @brief Writes the specified byte into the chunks code array. If the array
 * is not large enough, it will be resized by CHUNK_GROWTH_FACTOR.
 *
 * @param chunk Chunk* The chunk to write to.
 * @param byte uint8_t The byte to write.
 * @param line uint32_t The line number of the instruction.
 */
void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  // chunk does not have capacity to write to, so resize it
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

/**
 * @brief Adds a Value to the chunks constant array. 
 * 
 * @param chunk the chunk to add the value to.
 * @param value the value to add.
 * @return int 
 */
int addConstant(Chunk* chunk, Value value) {
    push(value);
    writeValueArray(&chunk->constants, value);
    pop();
    return chunk->constants.count - 1;
}

/**
 * @brief Frees a chunk by freeing its code array and zeroing out its memory.
 *
 * @param chunk Chunk* the chunk to free.
 */
void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}