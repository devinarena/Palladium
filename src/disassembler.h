
/**
* @file disassembler.h
* @author Devin Arena
* @brief Provides some debug functions for disassembling code.
* @since 6/22/2022
**/

#ifndef PALLADIUM_DISASSEMBLER_H
#define PALLADIUM_DISASSEMBLER_H

#include "chunk.h"
#include "commons.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif