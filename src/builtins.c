
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
#include "memory.h"
#include "object.h"

PdStruct* stl;
Value* pargv;

static Value p_clock(int argCount, Value* args) {
  return FROM_INTEGER(time(NULL));
}

static Value p_square(int argCount, Value* args) {
  int x = TO_INTEGER(args[0]);
  return FROM_INTEGER(x * x);
}

static Value p_atoi(int argCount, Value* args) {
  char* str = TO_CSTRING(args[0]);
  return FROM_INTEGER(atoi(str));
}

static Value write(int argCount, Value* args) {
  printValue(args[0]);
  printf("\n");
  return NULL_VAL;
}

static PdStruct* createSTLStruct(int argc, const char* argv[]) {
  PdStructTemplate* template = newStructTemplate();
  pargv = ALLOCATE(Value, argc);
  for (int i = 0; i < argc; i++) {
    pargv[i] = FROM_OBJECT(copyString(argv[i], strlen(argv[i])));
  }
  tableSet(&template->fieldTypes, copyString("argc", 4), FROM_INTEGER(argc));
  tableSet(&template->fieldTypes, copyString("argv", 4),
           (Value){.type = VALUE_POINTER,
                   .data.pointer = (struct Value*)pargv,
                   .pointerType = VALUE_OBJECT});
  tableSet(&template->fieldTypes, copyString("pi", 2),
           FROM_DOUBLE(3.14159265358979323846));
  tableSet(&template->fieldTypes, copyString("E", 1),
           FROM_DOUBLE(2.718281828459));

  PdString* p_write_name = copyString("write", 5);
  PdBuiltin* bin = newBuiltin(VALUE_NULL, &write, 1);
  INSERT_DYNAMIC_ARRAY(Value, bin->argt, FROM_OBJECT(p_write_name));
  tableSet(&template->fieldTypes, p_write_name, FROM_OBJECT(bin));
  return newStruct(template);
}

void initBuiltins(Table* globals, int argc, const char* argv[]) {
  stl = createSTLStruct(argc, argv);

  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(VALUE_INTEGER, &p_clock, 0)));

  PdBuiltin* bin_psquare = newBuiltin(VALUE_INTEGER, &p_square, 1);
  INSERT_DYNAMIC_ARRAY(Value, bin_psquare->argt, FROM_INTEGER(0));
  tableSet(globals, copyString("p_square", 8), FROM_OBJECT(bin_psquare));

  PdString* pdstr_patoi = copyString("p_atoi", 6);
  PdBuiltin* bin_patoi = newBuiltin(VALUE_INTEGER, &p_atoi, 1);
  INSERT_DYNAMIC_ARRAY(Value, bin_patoi->argt, FROM_OBJECT(pdstr_patoi));
  tableSet(globals, pdstr_patoi, FROM_OBJECT(bin_patoi));

  tableSet(globals, copyString("stl", 3), FROM_OBJECT(stl));
}

void freeBuiltins(Table* globals) {
  FREE(Value, pargv);
}