
/**
 * @file constant.h
 * @author Devin Arena
 * @brief Logic for constants in the program. A constant can be an integer,
 * boolean, double, or string.
 * @since 6/22/2022
 **/

#include "commons.h"

#ifndef PALLADIUM_VALUE_H
#define PALLADIUM_VALUE_H

#define IS_NULL(value) (value.type == VALUE_NULL)
#define IS_BOOL(value) (value.type == VALUE_BOOL)
#define IS_INTEGER(value) (value.type == VALUE_INTEGER)
#define IS_DOUBLE(value) (value.type == VALUE_DOUBLE)
#define IS_CHARACTER(value) (value.type == VALUE_CHARACTER)
#define IS_POINTER(value) (value.type == VALUE_POINTER)
#define IS_OBJECT(value) (value.type == VALUE_OBJECT)
#define IS_NUMBER(value) isNumberValue(value)
#define IS_NUMBER_TYPE(valueType) isNumberType(valueType)

#define TO_BOOL(value) (value.data.boolean)
#define TO_INTEGER(value) (value.data.integer)
#define TO_DOUBLE(value) (value.data.double_)
#define TO_CHARACTER(value) (value.data.character)
#define TO_POINTER(value) (Value*)(value.data.pointer)
#define TO_OBJECT(value) (value.data.object)

#define NULL_VAL ((Value){VALUE_NULL, {.integer = 0}})
#define NULL_POINTER ((Value){VALUE_POINTER, {.pointer = NULL}})
#define FROM_INTEGER(value) ((Value){VALUE_INTEGER, {.integer = value}})
#define FROM_DOUBLE(value) ((Value){VALUE_DOUBLE, {.double_ = value}})
#define FROM_BOOL(value) ((Value){VALUE_BOOL, {.boolean = value}})
#define FROM_CHARACTER(value) ((Value){VALUE_CHARACTER, {.character = value}})
#define FROM_POINTER(value) ((Value){VALUE_POINTER, {.pointer = (struct Value*)value}})
#define FROM_OBJECT(obj) ((Value){VALUE_OBJECT, {.object = (Object*)obj}})

// note: types must go in order of cast priority ascending
typedef enum {
  VALUE_NULL,
  VALUE_INTEGER,
  VALUE_DOUBLE,
  VALUE_BOOL,
  VALUE_CHARACTER,
  VALUE_POINTER,
  VALUE_OBJECT
} ValueType;

typedef struct Object Object;
typedef struct PdString PdString;

typedef struct {
  ValueType type;
  union {
    bool boolean;
    int integer;
    double double_;
    char character;
    // should look into this, these always point to the same thing. some optimizations could probably be made.
    struct Value* pointer;
    Object* object;
  } data;
  ValueType pointerType;
} Value;

void printValue(Value value);
bool valuesEqual(Value a, Value b);

static inline bool isNumberValue(Value value) {
  return IS_INTEGER(value) || IS_DOUBLE(value);
}

static inline bool isNumberType(ValueType type) {
  return type == VALUE_INTEGER || type == VALUE_DOUBLE;
}

#endif