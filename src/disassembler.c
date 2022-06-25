
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
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    // unary
    case OP_NOT_NUMBER:
      return simpleInstruction("OP_NOT_NUMBER", offset);
    case OP_NOT_BOOL:
      return simpleInstruction("OP_NOT_BOOL", offset);
    case OP_NEGATE_INT:
      return simpleInstruction("OP_NEGATE_INT", offset);
    case OP_NEGATE_DOUBLE:
      return simpleInstruction("OP_NEGATE_DOUBLE", offset);
    case OP_REFERENCE:
      return simpleInstruction("OP_REFERENCE", offset);
    case OP_DEREFERENCE:
      return simpleInstruction("OP_DEREFERENCE", offset);
      // binary
    case OP_ADD_INT:
      return simpleInstruction("OP_ADD_INT", offset);
    case OP_ADD_DOUBLE:
      return simpleInstruction("OP_ADD_DOUBLE", offset);
    case OP_SUB_INT:
      return simpleInstruction("OP_SUB_INT", offset);
    case OP_SUB_DOUBLE:
      return simpleInstruction("OP_SUB_DOUBLE", offset);
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
    default:
      printf("Unknown opcode encountered: %d", instruction);
      exit(1);
  }
}