
/**
 * @file chunk.h
 * @author Devin Arena
 * @brief Chunks contain a list of bytecode instructions processed by the VM.
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_CHUNK_H
#define PALLADIUM_CHUNK_H

#include "commons.h"
#include "dynamic_array.h"

// Helper macro for adding a constant to the correct dynamic array.
#define ADD_CONSTANT(chunk, type, value, index)                  \
  do {                                                           \
    INSERT_DYNAMIC_ARRAY(type, (&(chunk)->const_##type), value); \
    (*index) = (chunk)->const_##type.count - 1;                  \
  } while (false)

// VM Primitives
typedef enum { PRIMITIVE_INTEGER, PRIMITIVE_DOUBLE } PrimitiveType;

/**
 * @brief Chunks will store a list of bytecode instructions and constants. Each
 * function will eventually track its own chunk.
 */
typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  uint32_t* lines;
  DYNAMIC_ARRAY(int) const_int;
} Chunk;

void initChunk(Chunk* chunk);
void writeToChunk(Chunk* chunk, uint8_t byte, uint32_t line);
void freeChunk(Chunk* chunk);
void printConstant(Chunk* chunk, int index, PrimitiveType type);

// VM OpCodes
typedef enum { OP_RETURN, OP_CONSTANT, OP_PRINT } OpCode;

#endif