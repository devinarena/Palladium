
/**
 * @file vm.h
 * @author Devin Arena
 * @brief The Virtual Machine for Palladium. This is the main file for the
 * execution of user code.
 * @since 6/24/2022
 **/

#include "chunk.h"
#include "commons.h"
#include "dynamic_array.h"
#include "object.h"
#include "table.h"
#include "value.h"

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  DYNAMIC_ARRAY(Value) stack;
  Table strings;
  Table globals;
  Value* stackTop;
  Object* heap;
} VM;

extern VM vm;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();
Value peek(int distance);
void freeVM();