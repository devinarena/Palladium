
/**
 * @file value.h
 * @author Devin Arena
 * @brief Header for value representation
 * @since 5/20/2022
 **/

#ifndef PALLADIUM_VALUE_H
#define PALLADIUM_VALUE_H

#include <stdio.h>
#include <string.h>

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum { VAL_BOOL, VAL_NULL, VAL_NUMBER, VAL_OBJ } ValueType;

#ifdef NAN_BOXING

#define SIGN_BIT ((uint64_t)0x8000000000000000)
#define QNAN ((uint64_t)0x7ffc000000000000)
#define TAG_NULL 1   // 01
#define TAG_FALSE 2   // 10
#define TAG_TRUE 3  // 11

typedef uint64_t Value;

#define NUMBER_VAL(value) numToValue(value)
#define NULL_VAL ((Value)(uint64_t)(QNAN | TAG_NULL))
#define TRUE_VAL ((Value)(uint64_t)(QNAN | TAG_TRUE))
#define FALSE_VAL ((Value)(uint64_t)(QNAN | TAG_FALSE))
#define BOOL_VAL(value) ((value) ? TRUE_VAL : FALSE_VAL)
#define OBJ_VAL(obj) (Value)(SIGN_BIT | QNAN | (uint64_t)(uintptr_t)(obj))

#define IS_NUMBER(value) (((value)&QNAN) != QNAN)
#define IS_NULL(value) ((value) == NULL_VAL)
#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_OBJ(value) (((value) & (QNAN | SIGN_BIT)) == (QNAN | SIGN_BIT))

#define AS_NUMBER(value) valueToNum(value)
#define AS_BOOL(value) ((value) == TRUE_VAL)
#define AS_OBJ(value) ((Obj*)(uintptr_t)((value) & ~(SIGN_BIT | QNAN)))

static inline Value numToValue(double num) {
  Value value;
  // weird but should be optimized away
  memcpy(&value, &num, sizeof(Value));
  return value;
}

static inline double valueToNum(Value value) {
  double num;
  // weird but should be optimized away
  memcpy(&num, &value, sizeof(double));
  return num;
}

#else
typedef struct {
  ValueType type;
  union {
    bool boolean;
    double number;
    Obj* obj;
  } as;
} Value;

#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NULL_VAL ((Value){VAL_NULL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})

#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)
#define AS_OBJ(value) ((value).as.obj)

#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NULL(value) ((value).type == VAL_NULL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value) ((value).type == VAL_OBJ)

#endif

typedef struct {
  int capacity;
  int count;
  Value* values;
} ValueArray;

bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);

#endif