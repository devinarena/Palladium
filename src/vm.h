
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

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * sizeof(uint8_t))

typedef struct {
  Chunk* chunk;
  uint8_t* ip;
  Value* slot;
  ValueType returnType;
} CallFrame;

typedef struct {
  CallFrame callStack[FRAMES_MAX];
  int callStackSize;
  Value stack[STACK_MAX];
  Value* stackTop;
  Table strings;
  Table globals;
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
void swap();
void freeVM();