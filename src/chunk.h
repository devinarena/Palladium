
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
#include "value.h"

/**
 * @brief Chunks will store a list of bytecode instructions and constants. Each
 * function will eventually track its own chunk.
 */
typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  uint32_t* lines;
  DYNAMIC_ARRAY(Value) constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeToChunk(Chunk* chunk, uint8_t byte, uint32_t line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

// VM OpCodes
typedef enum {
  OP_RETURN,
  OP_NULL,
  OP_NULL_POINTER,
  OP_SWAP,
  OP_POP,
  OP_JUMP,
  OP_LOOP,
  OP_NOP,
  // Unary
  OP_NOT_NUMBER,
  OP_NOT_BOOL,
  OP_NEGATE_INT,
  OP_NEGATE_DOUBLE,
  OP_HEAP_REFERENCE,
  OP_STACK_REFERENCE,
  OP_DEREFERENCE,
  OP_JUMP_IF_FALSE,
  OP_JUMP_IF_TRUE,
  // Binary
  OP_ADD_INT,
  OP_ADD_DOUBLE,
  OP_ADD_POINTER,
  OP_ADD_OBJECT,
  OP_SUB_INT,
  OP_SUB_DOUBLE,
  OP_SUB_POINTER,
  OP_MUL_INT,
  OP_MUL_DOUBLE,
  OP_DIV_INT,
  OP_DIV_DOUBLE,
  OP_GREATER_INT,
  OP_GREATER_DOUBLE,
  OP_LESS_INT,
  OP_LESS_DOUBLE,
  OP_GREATER_EQUAL_INT,
  OP_GREATER_EQUAL_DOUBLE,
  OP_LESS_EQUAL_INT,
  OP_LESS_EQUAL_DOUBLE,
  OP_EQUALITY,
  // Constants
  OP_CONSTANT_INT,
  OP_CONSTANT_DOUBLE,
  OP_CONSTANT_BOOL,
  OP_CONSTANT_CHARACTER,
  OP_CONSTANT_STRING,
  OP_CONSTANT_POINTER,
  // Variables
  OP_GLOBAL_GET,
  OP_GLOBAL_SET,
  OP_GLOBAL_DEFINE,
  OP_LOCAL_SET,
  OP_LOCAL_GET,
  OP_STRUCT_INSTANCE,
  OP_STRUCT_GET,
  OP_STRUCT_SET,
  OP_MODULE_GET,
  OP_MODULE_SET,
  OP_ASSIGN,
  OP_PRINT,
  OP_OBJECT_CAST_PTR,
  OP_OBJECT_CAST,
  OP_ARITHMETIC_CAST_INT_DOUBLE,
  OP_ARITHMETIC_CAST_DOUBLE_INT,
  OP_ARITHMETIC_CAST_CHAR_INT,
  OP_ARITHMETIC_CAST_CHAR_DOUBLE,
  OP_ARITHMETIC_CAST_INT_CHAR,
  OP_POINTER_CAST,
  OP_INDEX,
  // Functions
  OP_CALL,
  OP_IMPORT
} OpCode;

#endif