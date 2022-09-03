
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
#include "object.h"

static Value pdclock(int argCount, Value* args) {
  return FROM_INTEGER(time(NULL));
}

static Value pdsquare(int argCount, Value* args) {
  int x = TO_INTEGER(args[0]);
  return FROM_INTEGER(x * x);
}

void initBuiltins(Table* globals) {
  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(VALUE_INTEGER, &pdclock, 0)));
  PdBuiltin* bin = newBuiltin(VALUE_INTEGER, &pdsquare, 1);
  INSERT_DYNAMIC_ARRAY(ValueType, bin->argt, VALUE_INTEGER);
  tableSet(globals, copyString("p_square", 8), FROM_OBJECT(bin));
}