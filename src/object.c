
/**
 * @file object.c
 * @author Devin Arena
 * @brief Object implementation for Palladium.
 * @since 5/25/2022
 **/

#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) \
  (type*)allocateObject(sizeof(type), objectType)

/**
 * @brief Allocates a basic object in memory and inserts it at the head of the
 * VM heap.
 *
 * @param size size_t the size of the object.
 * @param type ObjectType the type of the object.
 * @return Obj* the allocated object.
 */
static Obj* allocateObject(size_t size, ObjType type) {
  Obj* object = (Obj*)reallocate(NULL, 0, size);
  object->type = type;
  object->isMarked = false;

  object->next = vm.objects;
  vm.objects = object;

#ifdef DEBUG_LOG_GC
  printf("%p allocate %zu for %d\n", (void*)object, size, type);
#endif

  return object;
}

/**
 * @brief Allocates a new function object.
 *
 * @return ObjFunction* the allocated function object.
 */
ObjFunction* newFunction() {
  ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
  function->arity = 0;
  function->upvalueCount = 0;
  function->name = NULL;
  initChunk(&function->chunk);
  return function;
}

/**
 * @brief Allocates a new native function object.
 *
 * @param function NativeFunction the native function to wrap.
 * @return ObjNative* the allocated native function object.
 */
ObjNative* newNative(NativeFn function) {
  ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
  native->function = function;
  return native;
}

/**
 * @brief Allocates a new class object.
 *
 * @param name const char* the name of the class.
 * @return ObjClass* the allocated class object.
 */
ObjClass* newClass(ObjString* name) {
  ObjClass* clazz = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
  clazz->name = name;
  initTable(&clazz->methods);
  return clazz;
}

/**
 * @brief Allocates a new instance object.
 *
 * @param clazz ObjClass* the class of the instance.
 * @return ObjInstance* the allocated instance object.
 */
ObjInstance* newInstance(ObjClass* clazz) {
  ObjInstance* instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
  instance->clazz = clazz;
  initTable(&instance->fields);
  return instance;
}

/**
 * @brief Allocates a new upvalue object.
 *
 * @param closed Value* the closed value.
 * @return ObjUpvalue* the allocated upvalue object.
 */
ObjBoundMethod* newBoundMethod(Value receiver, ObjClosure* method) {
  ObjBoundMethod* boundMethod = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
  boundMethod->receiver = receiver;
  boundMethod->method = method;
  return boundMethod;
}

/**
 * @brief Allocates a new closure object.
 *
 * @param function ObjFunction* the function to bind.
 */
ObjClosure* newClosure(ObjFunction* function) {
  ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->upvalueCount);
  for (int i = 0; i < function->upvalueCount; i++) {
    upvalues[i] = NULL;
  }

  ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
  closure->function = function;
  closure->upvalues = upvalues;
  closure->upvalueCount = function->upvalueCount;
  return closure;
}

/**
 * @brief Allocates a new string object.
 *
 * @param chars const char* the characters of the string.
 * @param length int the length of the string.
 */
static ObjString* allocateString(char* chars, int length, uint32_t hash) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
  string->length = length;
  string->chars = chars;
  string->hash = hash;
  push(OBJ_VAL(string));
  tableSet(&vm.strings, string, NULL_VAL);
  pop();
  return string;
}

/**
 * @brief Hash function for a string. Runs a basic hash function on the string
 * and returns the hash.
 *
 * @param key const char* the characters of the string.
 * @param length int the length of the string.
 * @return uint32_t the hash of the string.
 */
static uint32_t hashString(const char* key, int length) {
  uint32_t hash = 2166136261u;

  for (int i = 0; i < length; i++) {
    hash ^= (uint8_t)key[i];
    hash *= 16777619;
  }

  return hash;
}

/**
 * @brief Allocates a string object with ownership of the C string.
 *
 * @param chars const char* the characters of the string.
 * @param length int the length of the string.
 * @return ObjString* the allocated string object.
 */
ObjString* takeString(char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

  if (interned != NULL) {
    FREE_ARRAY(char, chars, length + 1);
    return interned;
  }

  return allocateString(chars, length, hash);
}

/**
 * @brief Allocates a string object with a copy of the C string.
 *
 * @param chars const char* the characters of the string.
 * @param length int the length of the string.
 * @return ObjString* the allocated string object.
 */
ObjString* copyString(const char* chars, int length) {
  uint32_t hash = hashString(chars, length);
  ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
  if (interned != NULL)
    return interned;

  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';
  return allocateString(heapChars, length, hash);
}

/**
 * @brief Allocates a new upvalue object.
 *
 * @param slot Value* the slot to bind.
 * @return ObjUpvalue* the allocated upvalue object.
 */
ObjUpvalue* newUpvalue(Value* slot) {
  ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
  upvalue->location = slot;
  upvalue->next = NULL;
  upvalue->closed = NULL_VAL;
  return upvalue;
}

/**
 * @brief Prints the contents of a function <script> for top level, otherwise
 * the functions name.
 *
 * @param function ObjFunction* the function to print.
 */
static void printFunction(ObjFunction* function) {
  if (function->name == NULL) {
    printf("<script>");
    return;
  }
  printf("<function %s>", function->name->chars);
}

/**
 * @brief Handles printing dynamic objects.
 * 
 * @param value Value the value to print.
 */
void printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_FUNCTION: {
      printFunction(AS_FUNCTION(value));
      break;
    }
    case OBJ_NATIVE: {
      printf("<native fn>");
      break;
    }
    case OBJ_CLASS: {
      printf("<class %s>", AS_CLASS(value)->name->chars);
      break;
    }
    case OBJ_INSTANCE: {
      printf("<instance %s>", AS_INSTANCE(value)->clazz->name->chars);
      break;
    }
    case OBJ_BOUND_METHOD: {
      printFunction(AS_BOUND_METHOD(value)->method->function);
      break;
    }
    case OBJ_CLOSURE: {
      printFunction(AS_CLOSURE(value)->function);
      break;
    }
    case OBJ_STRING: {
      printf("%s", AS_CSTRING(value));
      break;
    }
    case OBJ_UPVALUE: {
      printf("upvalue");
      break;
    }
  }
}