
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

static Value p_clock(int argCount, Value* args) {
  return FROM_INTEGER(time(NULL));
}

static Value p_square(int argCount, Value* args) {
  int x = TO_INTEGER(args[0]);
  return FROM_INTEGER(x * x);
}

static Value PI = FROM_DOUBLE(3.141592654);

void initBuiltins(Table* globals, int argc, const char* argv[]) {
  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(VALUE_INTEGER, &p_clock, 0)));
  PdBuiltin* bin = newBuiltin(VALUE_INTEGER, &p_square, 1);
  INSERT_DYNAMIC_ARRAY(Value, bin->argt, FROM_INTEGER(0));
  tableSet(globals, copyString("p_square", 8), FROM_OBJECT(bin));
  tableSet(globals, copyString("PI", 2), PI);
  tableSet(globals,)
}