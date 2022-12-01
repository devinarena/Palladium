
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

static Value tostr(int argCount, Value* args) {
  return FROM_OBJECT(toString(&args[0]));
}

static Value readInt(int argCount, Value* args) {
  int i;
  scanf("%d", &i);
  return FROM_INTEGER(i);
}

static PdStruct* createSTLStruct(int argc, const char* argv[]) {
  PdStructTemplate* template = newStructTemplate();

  pargv = ALLOCATE(Value, argc);
  for (int i = 0; i < argc; i++) {
    pargv[i] = FROM_OBJECT(copyString(argv[i], strlen(argv[i])));
  }

  PdString* argc_str = copyString("argc", 4);
  tableSet(&template->fieldTypes, argc_str, FROM_INTEGER(argc));
  tableSet(&template->fieldIndices, argc_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* argv_str = copyString("argv", 4);
  tableSet(&template->fieldTypes, argv_str,
           (Value){.type = VALUE_POINTER,
                   .data.pointer = (struct Value*)pargv,
                   .pointerType = VALUE_POINTER});
  tableSet(&template->fieldIndices, argv_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* pi_str = copyString("pi", 2);
  tableSet(&template->fieldTypes, pi_str, FROM_DOUBLE(3.14159265358979323846));
  tableSet(&template->fieldIndices, pi_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* e_str = copyString("E", 1);
  tableSet(&template->fieldTypes, e_str, FROM_DOUBLE(2.718281828459));
  tableSet(&template->fieldIndices, e_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* p_write_name = copyString("write", 5);
  PdBuiltin* write_bin = newBuiltin(NULL_VAL, &write, 1);
  INSERT_DYNAMIC_ARRAY(Value, write_bin->argt, FROM_OBJECT(p_write_name));
  PdReference* write_ref = newReference(FROM_OBJECT(write_bin));
  tableSet(&template->fieldTypes, p_write_name, FROM_OBJECT(write_ref));
  tableSet(&template->fieldIndices, p_write_name,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* p_to_str = copyString("tostr", 5);
  PdBuiltin* to_str_bin = newBuiltin(FROM_OBJECT(p_to_str), &tostr, 1);
  INSERT_DYNAMIC_ARRAY(Value, to_str_bin->argt, NULL_VAL);
  PdReference* to_str_ref = newReference(FROM_OBJECT(to_str_bin));
  tableSet(&template->fieldTypes, p_to_str, FROM_OBJECT(to_str_ref));
  tableSet(&template->fieldIndices, p_to_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* psquare_str = copyString("square", 6);
  PdBuiltin* square_bin = newBuiltin(FROM_INTEGER(0), &p_square, 1);
  INSERT_DYNAMIC_ARRAY(Value, square_bin->argt, FROM_INTEGER(0));
  PdReference* square_ref = newReference(FROM_OBJECT(square_bin));
  tableSet(&template->fieldTypes, psquare_str, FROM_OBJECT(square_ref));
  tableSet(&template->fieldIndices, psquare_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* pdstr_patoi = copyString("atoi", 4);
  PdBuiltin* atoi_bin = newBuiltin(FROM_INTEGER(0), &p_atoi, 1);
  PdReference* atoi_ref = newReference(FROM_OBJECT(atoi_bin));
  INSERT_DYNAMIC_ARRAY(Value, atoi_bin->argt, FROM_OBJECT(pdstr_patoi));
  tableSet(&template->fieldTypes, pdstr_patoi, FROM_OBJECT(atoi_ref));
  tableSet(&template->fieldIndices, pdstr_patoi,
           FROM_INTEGER(template->fieldIndices.count));

  PdString* readint_str = copyString("readint", 7);
  PdBuiltin* readint_bin = newBuiltin(FROM_INTEGER(0), &readInt, 0);
  PdReference* readint_ref = newReference(FROM_OBJECT(readint_bin));
  tableSet(&template->fieldTypes, readint_str, FROM_OBJECT(readint_ref));
  tableSet(&template->fieldIndices, readint_str,
           FROM_INTEGER(template->fieldIndices.count));

  PdStruct* pstruct = newStruct(template);
  pstruct->memory->data[0] = FROM_INTEGER(argc);
  pstruct->memory->data[1] = (Value){.type = VALUE_POINTER,
                                     .data.pointer = (struct Value*)pargv,
                                     .pointerType = VALUE_POINTER};
  pstruct->memory->data[2] = FROM_DOUBLE(3.14159265358979323846);
  pstruct->memory->data[3] = FROM_DOUBLE(2.718281828459);
  pstruct->memory->data[4] = FROM_OBJECT(write_ref);
  pstruct->memory->data[5] = FROM_OBJECT(to_str_ref);
  pstruct->memory->data[6] = FROM_OBJECT(square_ref);
  pstruct->memory->data[7] = FROM_OBJECT(atoi_ref);
  pstruct->memory->data[8] = FROM_OBJECT(readint_ref);

  return pstruct;
}

void initBuiltins(Table* globals, int argc, const char* argv[]) {
  PdStruct* stl = createSTLStruct(argc, argv);

  tableSet(globals, copyString("clock", 5),
           FROM_OBJECT(newBuiltin(FROM_INTEGER(0), &p_clock, 0)));

  tableSet(globals, copyString("stl", 3), FROM_OBJECT(stl));
}

void freeBuiltins(Table* globals) {
  FREE(Value, pargv);
}