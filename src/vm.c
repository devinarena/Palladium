
/**
 * @file vm.c
 * @author Devin Arena
 * @brief Implementation of the virtual machine.
 * @since 6/24/2022
 **/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "disassembler.h"
#include "vm.h"

VM vm;

/**
 * @brief Resets the stack by pointing stackTop to the beginning (top) of the
 * stack.
 */
static void resetStack() {
  vm.stackTop = 0;
  vm.callStackSize = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  uint8_t instruction = vm.callStack->ip - vm.callStack->chunk->code;
  fprintf(stderr, "[line %d] in script.\n",
          vm.callStack->chunk->lines[instruction]);
  resetStack();
}

/**
 * @brief Initializes the VM by zeroing out memory.
 */
void initVM() {
  INIT_DYNAMIC_ARRAY(Value, vm.stack);
  vm.heap = NULL;
  initTable(&vm.strings);
  initTable(&vm.globals);
  resetStack();
}

/**
 * @brief Set the global in the VM's global table without caring for
 * redefinition.
 *
 * @param name PdString* the name of the global
 * @param value Value the value to set it to
 * @return bool true if the variable existed in the table previously, false
 * otherwise
 */
static bool setGlobal(PdString* name, Value value) {
  return !tableSet(&vm.globals, name, value);
}

/**
 * @brief Adds a global to the VM's global table. Reports a runtime error if the
 * global is already defined.
 *
 * @param name PdString* the name of the global
 * @param value Value the value to set it to
 */
static void addGlobal(PdString* name, Value value) {
  if (setGlobal(name, value)) {
    runtimeError("Global variable '%s' already defined.", name->chars);
  }
}

/**
 * @brief Calls a PdFunction by placing a CallFrame on top of the callstack
 * containing the opcodes for that function.
 *
 * @param function PdFunction* the function to call
 */
static void call(PdFunction* function, uint8_t argCount) {
  if (vm.callStackSize == 255) {
    runtimeError("Stack overflow.");
  }
  CallFrame frame;
  frame.chunk = &function->chunk;
  frame.ip = function->chunk.code;
  frame.slot = vm.stack.data + vm.stackTop - argCount;
  vm.callStack[vm.callStackSize] = frame;
  vm.callStackSize++;
#ifdef DEBUG_TRACE_EXEC
  printf("========= %s =========\n", function->name->chars);
#endif
}

/**
 * @brief Runs the chunk stored in the VM.
 *
 * @return InterpretResult The result of the execution.
 */
static InterpretResult run() {
  CallFrame* frame = &vm.callStack[vm.callStackSize - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (frame->ip += 2, (frame->ip[-2] << 8) | frame->ip[-1])
#define READ_CONSTANT() (frame->chunk->constants.data[READ_BYTE()])
#define READ_STRING() ((PdString*)TO_OBJECT(READ_CONSTANT()))
#define BINARY_OP(ctype, type, result, op) \
  do {                                     \
    ctype b = TO_##type(pop());            \
    ctype a = TO_##type(pop());            \
    push(FROM_##result(a op b));           \
  } while (false)

#define ARITHMETIC_OPS_NUM(op, symbol)           \
  {                                              \
    case OP_##op##_INT: {                        \
      BINARY_OP(int, INTEGER, INTEGER, symbol);  \
      break;                                     \
    }                                            \
    case OP_##op##_DOUBLE: {                     \
      BINARY_OP(double, DOUBLE, DOUBLE, symbol); \
      break;                                     \
    }                                            \
  }
#define ARITHMETIC_OPS_BOOL(op, symbol)        \
  {                                            \
    case OP_##op##_INT: {                      \
      BINARY_OP(int, INTEGER, BOOL, symbol);   \
      break;                                   \
    }                                          \
    case OP_##op##_DOUBLE: {                   \
      BINARY_OP(double, DOUBLE, BOOL, symbol); \
      break;                                   \
    }                                          \
  }

  while (true) {
    bool hitReturn = false;
    uint8_t traveled = 1;
    uint8_t instruction;
    switch ((instruction = READ_BYTE())) {
      case OP_RETURN: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
        printf("==================================\n");
#endif
        vm.callStackSize--;
        if (vm.callStackSize == 0) {
          return INTERPRET_OK;
        }
        int old = (vm.stack.data + vm.stackTop) - frame->slot;
        Value result;
        result.type = VALUE_NULL;
        if (vm.stackTop - old - 2 > 0) {
          result = pop();
        }
        frame = &vm.callStack[vm.callStackSize - 1];
        for (int i = 0; i < old + 1; i++) {
          pop();
        }
        if (result.type != VALUE_NULL)
          push(result);
        continue;
      }
      case OP_NULL: {
        push(NULL_VAL);
        break;
      }
      case OP_NULL_POINTER: {
        push(NULL_POINTER);
        break;
      }
      case OP_SWAP: {
        swap();
        break;
      }
      case OP_POP: {
        pop();
        break;
      }
      case OP_JUMP: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        frame->ip += offset;
        continue;
      }
      case OP_LOOP: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        frame->ip -= offset;
        continue;
      }
      // UNARY OPERATIONS
      case OP_NEGATE_INT: {
        push(FROM_INTEGER(-TO_INTEGER(pop())));
        break;
      }
      case OP_NEGATE_DOUBLE: {
        push(FROM_DOUBLE(-TO_DOUBLE(pop())));
        break;
      }
      case OP_NOT_NUMBER: {
        Value value = pop();
        push(FROM_BOOL(!(value.data.integer > 0 || value.data.double_ > 0)));
        break;
      }
      case OP_NOT_BOOL: {
        push(FROM_BOOL(!TO_BOOL(pop())));
        break;
      }
      case OP_REFERENCE: {
        Value value = pop();
        Value pointer = FROM_POINTER((struct Value*)&value);
        push(pointer);
        break;
      }
      case OP_DEREFERENCE: {
        Value value = pop();
        if (value.data.pointer == NULL) {
          runtimeError("Dereferencing NULL pointer.");
          exit(1);
        }
        Value deref = *(Value*)value.data.pointer;
        push(deref);
        break;
      }
      case OP_ARITHMETIC_CAST_INT_DOUBLE: {
        Value int_ = pop();
        push(FROM_DOUBLE((double)TO_INTEGER(int_)));
        break;
      }
      case OP_JUMP_IF_FALSE: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        Value condition = peek(0);
        if (!TO_BOOL(condition)) {
          frame->ip += offset;
        }
        continue;
      }
      case OP_JUMP_IF_TRUE: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        Value condition = peek(0);
        if (TO_BOOL(condition)) {
          frame->ip += offset;
        }
        continue;
      }
        // BINARY OPERATIONS
        ARITHMETIC_OPS_NUM(ADD, +)
        ARITHMETIC_OPS_NUM(SUB, -)
        ARITHMETIC_OPS_NUM(MUL, *)
        ARITHMETIC_OPS_NUM(DIV, /)
      case OP_ADD_POINTER: {
        break;
      }
      case OP_SUB_POINTER: {
        break;
      }
        ARITHMETIC_OPS_BOOL(GREATER, >)
        ARITHMETIC_OPS_BOOL(LESS, <)
        ARITHMETIC_OPS_BOOL(GREATER_EQUAL, >=)
        ARITHMETIC_OPS_BOOL(LESS_EQUAL, <=)
      case OP_EQUALITY: {
        Value b = pop();
        Value a = pop();
        push(FROM_BOOL(valuesEqual(a, b)));
        break;
      }
      // CONSTANTS
      case OP_CONSTANT_INT:
      case OP_CONSTANT_BOOL:
      case OP_CONSTANT_CHARACTER:
      case OP_CONSTANT_STRING:
      case OP_CONSTANT_POINTER:
      case OP_CONSTANT_DOUBLE: {
        Value constant = READ_CONSTANT();
        push(constant);
        traveled++;
        break;
      }
      // Variables
      case OP_GLOBAL_GET: {
        PdString* name = READ_STRING();
        Value value;
        if (!tableGet(&vm.globals, name, &value)) {
          runtimeError("Undefined variable '%s'.", name->chars);
          return INTERPRET_RUNTIME_ERROR;
        }
        push(value);
        traveled++;
        break;
      }
      case OP_GLOBAL_SET: {
        PdString* name = READ_STRING();
        Value value = pop();
        setGlobal(name, value);
        traveled++;
        break;
      }
      case OP_GLOBAL_DEFINE: {
        PdString* name = READ_STRING();
        Value value = pop();
        addGlobal(name, value);
        traveled++;
        break;
      }
      case OP_LOCAL_GET: {
        uint8_t slot = READ_BYTE();
        Value value = frame->slot[slot];
        push(value);
        traveled++;
        break;
      }
      case OP_LOCAL_SET: {
        uint8_t slot = READ_BYTE();
        frame->slot[slot] = peek(0);
        traveled++;
        break;
      }
      case OP_PRINT: {
        Value value = pop();
        printValue(value);
        printf("\n");
        break;
      }
      // Function calls
      case OP_CALL: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(
            frame->chunk, (int)(frame->ip - traveled - frame->chunk->code));
#endif
        uint8_t argCount = READ_BYTE();
        Value fun = peek(argCount);
        if (!IS_OBJECT(fun) || TO_OBJECT(fun)->type != ObjectFunction) {
          runtimeError("Cannot call non-function.");
          return INTERPRET_RUNTIME_ERROR;
        }
        PdFunction* fn = TO_FUNCTION(fun);
        if (argCount != fn->arity) {
          runtimeError("Expected %d arguments but got %d.", fn->arity,
                       argCount);
          return INTERPRET_RUNTIME_ERROR;
        }
        for (int i = vm.stackTop - argCount - 1; i < vm.stackTop - 1; i++) {
          if (peek(i).type != fn->locals.data[i - vm.stackTop + argCount + 1]) {
            runtimeError("Mismatched argument type for position %d.", (vm.stackTop - 1) - i - 1);
            return INTERPRET_RUNTIME_ERROR;
          }
        }
        call(fn, argCount);
        frame = &vm.callStack[vm.callStackSize - 1];
        continue;
      }
    }
      // I like to see the stack after the operation happens
#ifdef DEBUG_TRACE_EXEC
    disassembleInstruction(frame->chunk,
                           (int)(frame->ip - traveled - frame->chunk->code));
    printf("        ");
    for (int slot = 0; slot < vm.stackTop; slot++) {
      printf("[");
      printValue(vm.stack.data[slot]);
      printf("]");
    }
    printf("\n");
#endif
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
#undef ARITHMETIC_OPS
}

/**
 * @brief Interprets code given by a source string.
 *
 * @param source const char* The source code to interpret.
 * @return InterpretResult the result of the interpretation.
 */
InterpretResult interpret(const char* source) {
  initVM();

  PdFunction* fun = compile(source);

  if (!fun) {
    return INTERPRET_COMPILE_ERROR;
  }

  call(fun, 0);

  InterpretResult res = run();

  freeVM();

  return res;
}

/**
 * @brief Pushes value onto the stack and increments stackTop.
 *
 * @param value Value the value to push.
 */
void push(Value value) {
  INSERT_DYNAMIC_ARRAY(Value, vm.stack, value);
  vm.stackTop++;
}

/**
 * @brief Pops the top value off of the stack and returns it.
 *
 * @return Value the value popped off of the stack.
 */
Value pop() {
  vm.stack.data[vm.stack.count--] = NULL_VAL;
  vm.stackTop--;
  return vm.stack.data[vm.stackTop];
}

/**
 * @brief Peeks at the value {distance} positions from the top of the stack.
 *
 * @param distance int The distance from the top of the stack to peek at.
 * @return Value The value at the given distance from the top of the stack.
 */
Value peek(int distance) {
  return vm.stack.data[vm.stackTop - 1 - distance];
}

/**
 * @brief Swaps the two top values on the stack.
 */
void swap() {
  Value a = pop();
  Value b = pop();
  push(a);
  push(b);
}

/**
 * @brief Frees any memory allocated by the VM.
 */
void freeVM() {
  FREE_DYNAMIC_ARRAY(Value, vm.stack);

  freeTable(&vm.globals);
  freeTable(&vm.strings);
  Object* current = vm.heap;
  while (current != NULL) {
    Object* next = current->next;
    freeObject(current);
    current = next;
  }
}