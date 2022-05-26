
/**
 * @file compiler.h
 * @author Devin Arena
 * @brief Compiler for Palladium.
 * @since 5/20/2022
 **/

#ifndef PALLADIUM_COMPILER_H
#define PALLADIUM_COMPILER_H

#include <stdlib.h>

#include "chunk.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

bool compile(const char* source, Chunk* chunk);

#endif