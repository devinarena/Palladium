
/**
 * @file disassembler.c
 * @author Devin Arena
 * @brief Provides some debug functions for disassembling code.
 * @since 6/22/2022
 **/

#include <stdio.h>
#include <stdlib.h>

#include "disassembler.h"
#include "value.h"

/**
 * @brief Disassembles an entire chunk of code.
 *
 * @param chunk Chunk* the chunk to disassemble.
 * @param name const char* the name of the chunk.
 */
void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);
  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
  printf("\n");
}

/**
 * @brief Helper for displaying a simple instruction and moving the offset to
 * the next instruction.
 *
 * @param name const char* the name of the instruction
 * @param offset int the offset of the instruction
 * @return int the offset of the next instruction
 */
static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

/**
 * @brief Helper for displaying a byte instruction, an instruction followed by a
 * single byte.
 *
 * @param name const char* the name of the instruction
 * @param chunk Chunk* the chunk to disassemble
 * @param offset int the offset of the instruction
 * @return int the offset of the next instruction
 */
static int byteInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t data = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, data);
  return offset + 2;
}

/**
 * @brief Helper for displaying a short instruction and moving the offset to
 * the next instruction.
 *
 * @param name const char* the name of the instruction
 * @param chunk Chunk* the chunk being disassembled
 * @param offset the offset of the instruction
 * @return int the offset of the next instruction
 */
static int shortInstruction(const char* name, Chunk* chunk, int offset) {
  uint16_t data = (chunk->code[offset + 1] << 8) | chunk->code[offset + 2];
  printf("%-16s %4d\n", name, data);
  return offset + 3;
}

/**
 * @brief Helper for displaying a constant instruction and moving the offset to
 * the next instruction.
 *
 * @param name const char* the name of the instruction
 * @param chunk Chunk* the chunk being disassembled
 * @param offset the offset of the instruction
 * @return int the offset of the next instruction
 */
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.data[constant]);
  printf("'\n");
  return offset + 2;
}

/**
 * @brief Disassembles an instruction at the specified offset, printing it
 * to standard output.
 *
 * @param chunk Chunk* The chunk we are disassembling.
 * @param offset the offset of the instruction to disassemble.
 * @return int the offset of the next instruction.
 */
int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1])
    printf("   | ");
  else
    printf("%4d ", chunk->lines[offset]);

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_NULL:
      return simpleInstruction("OP_NULL", offset);
    case OP_NULL_POINTER:
      return simpleInstruction("OP_NULL_POINTER", offset);
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OP_SWAP:
      return simpleInstruction("OP_SWAP", offset);
    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_JUMP:
      return shortInstruction("OP_JUMP", chunk, offset);
    case OP_LOOP:
      return shortInstruction("OP_LOOP", chunk, offset);
    // unary
    case OP_NOT_NUMBER:
      return simpleInstruction("OP_NOT_NUMBER", offset);
    case OP_NOT_BOOL:
      return simpleInstruction("OP_NOT_BOOL", offset);
    case OP_NEGATE_INT:
      return simpleInstruction("OP_NEGATE_INT", offset);
    case OP_NEGATE_DOUBLE:
      return simpleInstruction("OP_NEGATE_DOUBLE", offset);
    case OP_HEAP_REFERENCE:
      return simpleInstruction("OP_HEAP_REFERENCE", offset);
    case OP_STACK_REFERENCE:
      return simpleInstruction("OP_STACK_REFERENCE", offset);
    case OP_DEREFERENCE:
      return simpleInstruction("OP_DEREFERENCE", offset);
    case OP_JUMP_IF_FALSE:
      return shortInstruction("OP_JUMP_IF_FALSE", chunk, offset);
    case OP_JUMP_IF_TRUE:
      return shortInstruction("OP_JUMP_IF_TRUE", chunk, offset);
      // binary
    case OP_ADD_INT:
      return simpleInstruction("OP_ADD_INT", offset);
    case OP_ADD_DOUBLE:
      return simpleInstruction("OP_ADD_DOUBLE", offset);
    case OP_ADD_POINTER:
      return simpleInstruction("OP_ADD_POINTER", offset);
    case OP_ADD_OBJECT:
      return simpleInstruction("OP_ADD_OBJECT", offset);
    case OP_SUB_INT:
      return simpleInstruction("OP_SUB_INT", offset);
    case OP_SUB_DOUBLE:
      return simpleInstruction("OP_SUB_DOUBLE", offset);
    case OP_SUB_POINTER:
      return simpleInstruction("OP_SUB_POINTER", offset);
    case OP_MUL_INT:
      return simpleInstruction("OP_MUL_INT", offset);
    case OP_MUL_DOUBLE:
      return simpleInstruction("OP_MUL_DOUBLE", offset);
    case OP_DIV_INT:
      return simpleInstruction("OP_DIV_INT", offset);
    case OP_DIV_DOUBLE:
      return simpleInstruction("OP_DIV_DOUBLE", offset);
    case OP_EQUALITY:
      return simpleInstruction("OP_EQUALITY", offset);
    case OP_GREATER_INT:
      return simpleInstruction("OP_GREATER_INT", offset);
    case OP_GREATER_DOUBLE:
      return simpleInstruction("OP_GREATER_DOUBLE", offset);
    case OP_LESS_INT:
      return simpleInstruction("OP_LESS_INT", offset);
    case OP_LESS_DOUBLE:
      return simpleInstruction("OP_LESS_DOUBLE", offset);
    case OP_GREATER_EQUAL_INT:
      return simpleInstruction("OP_GREATER_EQUAL_INT", offset);
    case OP_GREATER_EQUAL_DOUBLE:
      return simpleInstruction("OP_GREATER_EQUAL_DOUBLE", offset);
    case OP_LESS_EQUAL_INT:
      return simpleInstruction("OP_LESS_EQUAL_INT", offset);
    case OP_LESS_EQUAL_DOUBLE:
      return simpleInstruction("OP_LESS_EQUAL_DOUBLE", offset);
      // constants
    case OP_CONSTANT_INT:
      return constantInstruction("OP_CONSTANT_INT", chunk, offset);
    case OP_CONSTANT_DOUBLE:
      return constantInstruction("OP_CONSTANT_DOUBLE", chunk, offset);
    case OP_CONSTANT_BOOL:
      return constantInstruction("OP_CONSTANT_BOOL", chunk, offset);
    case OP_CONSTANT_CHARACTER:
      return constantInstruction("OP_CONSTANT_CHARACTER", chunk, offset);
    case OP_CONSTANT_STRING:
      return constantInstruction("OP_CONSTANT_STRING", chunk, offset);
    case OP_CONSTANT_POINTER:
      return constantInstruction("OP_CONSTANT_POINTER", chunk, offset);
    case OP_ARITHMETIC_CAST_CHAR_INT:
      return simpleInstruction("OP_ARITHMETIC_CAST_CHAR_INT", offset);
    case OP_ARITHMETIC_CAST_CHAR_DOUBLE:
      return simpleInstruction("OP_ARITHMETIC_CAST_CHAR_DOUBLE", offset);
    case OP_ARITHMETIC_CAST_INT_DOUBLE:
      return simpleInstruction("OP_ARITHMETIC_CAST_INT_DOUBLE", offset);
    case OP_ARITHMETIC_CAST_INT_CHAR:
      return simpleInstruction("OP_ARITHMETIC_CAST_INT_CHAR", offset);
    case OP_ARITHMETIC_CAST_DOUBLE_INT:
      return simpleInstruction("OP_ARITHMETIC_CAST_DOUBLE_INT", offset);
    case OP_POINTER_CAST:
      return constantInstruction("OP_POINTER_CAST", chunk, offset);
    case OP_OBJECT_CAST:
      return simpleInstruction("OP_OBJECT_CAST", offset);
    case OP_OBJECT_CAST_PTR:
      return simpleInstruction("OP_OBJECT_CAST_PTR", offset);
    case OP_GLOBAL_GET:
      return constantInstruction("OP_GLOBAL_GET", chunk, offset);
    case OP_GLOBAL_SET:
      return constantInstruction("OP_GLOBAL_SET", chunk, offset);
    case OP_GLOBAL_DEFINE:
      return constantInstruction("OP_GLOBAL_DEFINE", chunk, offset);
    case OP_LOCAL_GET:
      return byteInstruction("OP_LOCAL_GET", chunk, offset);
    case OP_LOCAL_SET:
      return byteInstruction("OP_LOCAL_SET", chunk, offset);
    case OP_STRUCT_INSTANCE:
      return simpleInstruction("OP_STRUCT_INSTANCE", offset);
    case OP_STRUCT_GET:
      return constantInstruction("OP_STRUCT_GET", chunk, offset);
    case OP_STRUCT_SET:
      return constantInstruction("OP_STRUCT_SET", chunk, offset);
    case OP_MODULE_GET:
      return constantInstruction("OP_MODULE_GET", chunk, offset);
    case OP_MODULE_SET:
      return constantInstruction("OP_MODULE_SET", chunk, offset);
    case OP_ASSIGN:
      return simpleInstruction("OP_ASSIGN", offset);
    case OP_NOP:
      return simpleInstruction("OP_NOP", offset);
    // Function calls
    case OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case OP_IMPORT:
      return constantInstruction("OP_IMPORT", chunk, offset);
    default:
      printf("Unknown opcode encountered: %d", instruction);
      exit(1);
  }
}