
/**
* @file debug.h
* @author Devin Arena
* @brief Helper methods for debugging purposes.
* @since 5/20/2022
**/

#ifndef PALLADIUM_DEBUG_H
#define PALLADIUM_DEBUG_H

#include "chunk.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif