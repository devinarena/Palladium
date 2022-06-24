/**
 * @file debug.c
 * @author Devin Arena
 * @brief Debug features implementation file.
 * @since 5/20/2022
 **/

#include <stdio.h>

#include "debug.h"
#include "object.h"
#include "value.h"

/**
 * @brief Disassembles a chunk instruction by instruction, printing the opcode
 * and operands.
 *
 * @param chunk Chunk* the chunk being disassembled.
 * @param name const char* the name of the chunk.
 */
void disassembleChunk(Chunk* chunk, const char* name) {
  printf("== %s ==\n", name);

  for (int offset = 0; offset < chunk->count;) {
    offset = disassembleInstruction(chunk, offset);
  }
}

/**
 * @brief Simply displays the name of an instruction.
 *
 * @param name const char* the name of the instruction.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
static int simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return offset + 1;
}

/**
 * @brief Displays the name of an instruction and a one byte operand.
 *
 * @param name const char* the name of the instruction.
 * @param chunk Chunk* the chunk being disassembled.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
static int byteInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t slot = chunk->code[offset + 1];
  printf("%-16s %4d\n", name, slot);
  return offset + 2;
}

/**
 * @brief Constant instructions also show the name, index, and value of the
 * constant.
 *
 * @param name const char* the name of the instruction.
 * @param chunk Chunk* the chunk being disassembled.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
static int constantInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");

  return offset + 2;
}

/**
 * @brief Displays the name and operands as well as argument count for an invoke
 * instruction.
 *
 * @param name const char* the name of the instruction.
 * @param chunk Chunk* the chunk being disassembled.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
static int invokeInstruction(const char* name, Chunk* chunk, int offset) {
  uint8_t constant = chunk->code[offset + 1];
  uint8_t argCount = chunk->code[offset + 2];

  printf("%-16s (%d args) %4d '", name, argCount, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 3;
}

/**
 * @brief Displays the name and two byte position operands for a jump
 * instruction.
 *
 * @param name const char* the name of the instruction.
 * @param sign int the sign of the jump.
 * @param chunk Chunk* the chunk being disassembled.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
static int jumpInstruction(const char* name,
                           int sign,
                           Chunk* chunk,
                           int offset) {
  uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
  jump |= chunk->code[offset + 2];
  printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
  return offset + 3;
}

/**
 * @brief Disassembles a single instruction, printing it in a nicely formatted
 * way.
 *
 * @param chunk Chunk* the chunk being disassembled.
 * @param offset the offset of the instruction.
 * @return int the offset of the next instruction.
 */
int disassembleInstruction(Chunk* chunk, int offset) {
  printf("%04d ", offset);

  if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", chunk->lines[offset]);
  }

  uint8_t instruction = chunk->code[offset];
  switch (instruction) {
    case OP_PRINT:
      return simpleInstruction("OP_PRINT", offset);
    case OP_JUMP:
      return jumpInstruction("OP_JUMP", 1, chunk, offset);
    case OP_JUMP_IF_FALSE:
      return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
    case OP_JUMP_IF_TRUE:
      return jumpInstruction("OP_JUMP_IF_TRUE", 1, chunk, offset);
    case OP_LOOP:
      return jumpInstruction("OP_LOOP", -1, chunk, offset);
    case OP_CALL:
      return byteInstruction("OP_CALL", chunk, offset);
    case OP_CLOSURE: {
      offset++;
      uint8_t constant = chunk->code[offset++];
      printf("%-16s %4d ", "OP_CLOSURE", constant);
      printValue(chunk->constants.values[constant]);
      printf("\n");

      ObjFunction* func = AS_FUNCTION(chunk->constants.values[constant]);
      for (int i = 0; i < func->upvalueCount; i++) {
        int isLocal = chunk->code[offset++];
        int index = chunk->code[offset++];
        printf("%-16s %4d %s %4d\n", "OP_CLOSURE", offset - 2,
               isLocal ? "local" : "upvalue", index);
      }
      return offset;
    }
    case OP_CLOSE_UPVALUE:
      return simpleInstruction("OP_CLOSE_UPVALUE", offset);
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
    case OP_CONSTANT:
      return constantInstruction("OP_CONSTANT", chunk, offset);
    case OP_NULL:
      return simpleInstruction("OP_NULL", offset);
    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);
    case OP_POP:
      return simpleInstruction("OP_POP", offset);
    case OP_GET_LOCAL:
      return byteInstruction("OP_GET_LOCAL", chunk, offset);
    case OP_SET_LOCAL:
      return byteInstruction("OP_SET_LOCAL", chunk, offset);
    case OP_GET_GLOBAL:
      return constantInstruction("OP_GET_GLOBAL", chunk, offset);
    case OP_DEFINE_GLOBAL:
      return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);
    case OP_SET_GLOBAL:
      return constantInstruction("OP_SET_GLOBAL", chunk, offset);
    case OP_GET_UPVALUE:
      return byteInstruction("OP_GET_UPVALUE", chunk, offset);
    case OP_SET_UPVALUE:
      return byteInstruction("OP_SET_UPVALUE", chunk, offset);
    case OP_GET_PROPERTY:
      return constantInstruction("OP_GET_PROPERTY", chunk, offset);
    case OP_SET_PROPERTY:
      return constantInstruction("OP_SET_PROPERTY", chunk, offset);
    case OP_EQUAL:
      return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
      return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
      return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
      return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    case OP_CLASS:
      return constantInstruction("OP_CLASS", chunk, offset);
    case OP_METHOD:
      return constantInstruction("OP_METHOD", chunk, offset);
    case OP_INVOKE:
      return invokeInstruction("OP_INVOKE", chunk, offset);
    case OP_INHERIT:
      return simpleInstruction("OP_INHERIT", offset);
    case OP_SUPER_GET:
      return constantInstruction("OP_GET_SUPER", chunk, offset);
    case OP_SUPER_INVOKE:
      return invokeInstruction("OP_INVOKE_SUPER", chunk, offset);
    default:
      printf("Unknown opcode %d", instruction);
      return offset + 1;
  }
}