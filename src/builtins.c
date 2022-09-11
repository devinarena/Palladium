
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

static PdStruct* createSTLStruct(int argc, const char* argv[]) {
  PdStructTemplate* template = newStructTemplate();
  PdString pargv[argc];
  for (int i = 0; i < argc; i++) {
    pargv[i] = *copyString(argv[i], strlen(argv[i]));
  }
  tableSet(&template->fieldTypes, copyString("argc", 4), FROM_INTEGER(argc));
  tableSet(&template->fieldTypes, copyString("argv", 4), FROM_POINTER(&pargv));
  tableSet(&template->fieldTypes, copyString("pi", 2), FROM_DOUBLE(3.14159265358979323846));
  return newStruct(template);
}

void initBuiltins(Table* globals, int argc, const char* argv[]) {
  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(VALUE_INTEGER, &p_clock, 0)));
  PdBuiltin* bin = newBuiltin(VALUE_INTEGER, &p_square, 1);
  INSERT_DYNAMIC_ARRAY(Value, bin->argt, FROM_INTEGER(0));
  tableSet(globals, copyString("p_square", 8), FROM_OBJECT(bin));
  tableSet(globals, copyString("stl", 3),
           FROM_OBJECT(createSTLStruct(argc, argv)));
}