
/**
 * @file object.h
 * @author Devin Arena
 * @brief Object representation in Palladium.
 * @since 6/24/2022
 **/

#ifndef PALLADIUM_OBJECT_H
#define PALLADIUM_OBJECT_H

#include "commons.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value) (((Object*)value.data.object)->type)

#define TO_OBJECT(value) (value.data.object)

#define TO_STRING(value) ((PdString*)value.data.object)
#define TO_CSTRING(value) ((TO_STRING(value)->chars))
#define TO_FUNCTION(value) ((PdFunction*)value.data.object)

typedef enum {
  ObjectString,
  ObjectFunction
} ObjectType;

struct Object {
  ObjectType type;
  struct Object* next;  // heap
};

struct PdString {
  Object object;
  uint32_t length;
  char* chars;
  uint32_t hash;
};

typedef struct {
  Object object;
  Chunk chunk;
  uint8_t arity;
  DYNAMIC_ARRAY(ValueType) locals;
  ValueType returnType;
  PdString* name;
} PdFunction;

PdString* newString(char* chars, int length);
PdString* copyString(const char* chars, int length);
PdFunction* newFunction(ValueType returnType, PdString* name);
void printObject(Value value);

#endif