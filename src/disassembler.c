
/**
 * @file disassembler.c
 * @author Devin Arena
 * @brief Provides some debug functions for disassembling code.
 * @since 6/22/2022
 **/

#include <stdio.h>
#include <stdlib.h>

#include "disassembler.h"

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
 * @param chunk Chunk* the chunk being disassembled
 * @param offset the offset of the instruction
 * @return int the offset of the next instruction
 */
static int constantInstruction(Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", "OP_CONSTANT", constant);
  printConstant(chunk, constant, PRIMITIVE_INTEGER);
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
    case OP_CONSTANT:
      return constantInstruction(chunk, offset);
    default:
      printf("Unknown opcode encountered: %d", instruction);
      exit(1);
  }
}