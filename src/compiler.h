
/**
* @file compiler.h
* @author Devin Arena
* @brief Compiles tokens parsed by the scanner into opcodes.
* @since 6/22/2022
**/

#ifndef PALLADIUM_COMPILER_H
#define PALLADIUM_COMPILER_H

#include "commons.h"
#include "scanner.h"
#include "chunk.h"

bool compile(const char* source, Chunk* chunk);

#endif