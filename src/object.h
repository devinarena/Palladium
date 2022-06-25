
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

#define OBJ_TYPE(value) (((Object*)value.data.object)->type)

#define TO_OBJECT(value) (value.data.object)

#define TO_CSTRING(value) (((ObjString*)value.data.object)->chars)

typedef enum {
  ObjectString,
} ObjectType;

struct Object {
  ObjectType type;
  struct Object* next;  // heap
};

struct ObjString {
  Object object;
  uint32_t length;
  char* chars;
  uint32_t hash;
};

ObjString* newString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

#endif