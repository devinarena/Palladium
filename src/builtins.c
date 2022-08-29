
/**
 * @file builtins.h
 * @author Devin Arena
 * @brief Builtin functions implementation for Palladium.
 * @since 8/29/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "builtins.h"

static Value pdclock() {
  return FROM_INTEGER(time(NULL));
}

void initBuiltins(Table* globals) {
  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(VALUE_INTEGER, &pdclock)));
}