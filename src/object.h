
/**
 * @file object.h
 * @author Devin Arena
 * @brief Object representation in Palladium.
 * @since 6/24/2022
 **/

#ifndef PALLADIUM_OBJECT_H
#define PALLADIUM_OBJECT_H

#include "chunk.h"
#include "commons.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value) (((Object*)value.data.object)->type)

#define TO_OBJECT(value) (value.data.object)

#define TO_STRING(value) ((PdString*)value.data.object)
#define TO_CSTRING(value) ((TO_STRING(value)->chars))
#define TO_FUNCTION(value) ((PdFunction*)value.data.object)
#define TO_BUILTIN(value) ((PdBuiltin*)value.data.object)
#define TO_STRUCT_TEMPLATE(value) ((PdStructTemplate*)value.data.object)
#define TO_STRUCT(value) ((PdStruct*)value.data.object)
#define TO_REFERENCE(value) ((PdReference*)value.data.object)
#define TO_MODULE(value) ((PdModule*)value.data.object)

typedef Value (*NativeFn)(int argCount, Value* args);

typedef enum {
  ObjectString,
  ObjectFunction,
  ObjectBuiltin,
  ObjectStructTemplate,
  ObjectStruct,
  ObjectReference,
  ObjectModule
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
  DYNAMIC_ARRAY(Value) locals;
  ValueType returnType;
  PdString* name;
} PdFunction;

typedef struct {
  Object object;
  uint8_t arity;
  DYNAMIC_ARRAY(Value) argt;
  NativeFn builtinRef;
  Value returnType;
} PdBuiltin;

typedef struct {
  Object object;
  Table fieldTypes;
  Table fieldIndices;
} PdStructTemplate;

typedef struct {
  Object object;
  PdStructTemplate* template;
  Value* fields;
  int fieldCount;
} PdStruct;

typedef struct {
  Object object;
  Value value;
} PdReference;

typedef struct {
  Object object;
  Table globals;
  uint8_t nameIndex;
  uint8_t index;
  struct PdModule* parent;
} PdModule;

PdString* newString(char* chars, int length);
PdString* copyString(const char* chars, int length);
PdFunction* newFunction(ValueType returnType, PdString* name);
PdBuiltin* newBuiltin(Value returnType, NativeFn builtinRef, int arity);
PdStructTemplate* newStructTemplate();
PdStruct* newStruct(PdStructTemplate* template);
PdReference* newReference(Value value);
PdModule* newModule();
void printObject(Value value);
const char* getObjectTypeName(ObjectType type);

#endif