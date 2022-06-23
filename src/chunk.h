
/**
 * @file chunk.h
 * @author Devin Arena
 * @brief Chunks contain a list of bytecode instructions processed by the VM.
 * @since 6/22/2022
 **/

#ifndef PALLADIUM_CHUNK_H
#define PALLADIUM_CHUNK_H

#include "common.h"
#include "value.h"
// VM Operations
typedef enum {
  OP_NULL,
  OP_FALSE,
  OP_TRUE,
  OP_CONSTANT,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_GET_PROPERTY,
  OP_SET_PROPERTY,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_JUMP_IF_TRUE,
  OP_LOOP,
  OP_CALL,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN,
  OP_CLASS,
  OP_METHOD,
  OP_INVOKE,
  OP_INHERIT,
  OP_SUPER_GET,
  OP_SUPER_INVOKE
} OpCode;

typedef struct {
  int count;
  int capacity;
  uint8_t* code;
  int* lines;
  ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif