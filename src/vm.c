
/**
 * @file vm.c
 * @author Devin Arena
 * @brief Implementation of the virtual machine.
 * @since 6/24/2022
 **/

#include <stdarg.h>
#include <stdio.h>

#include "compiler.h"
#include "disassembler.h"
#include "vm.h"

VM vm;

/**
 * @brief Resets the stack by pointing stackTop to the beginning (top) of the
 * stack.
 */
static void resetStack() {
  vm.stackTop = vm.stack.data;
  vm.callStackSize = 0;
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  uint8_t instruction = vm.callStack->ip - vm.callStack->chunk->code;
  fprintf(stderr, "[line %d] in script.\n", vm.callStack->chunk->lines[instruction]);
  resetStack();
}

/**
 * @brief Initializes the VM by zeroing out memory.
 */
void initVM() {
  INIT_DYNAMIC_ARRAY(Value, vm.stack);
  vm.heap = NULL;
  vm.callStack = 0;
  initTable(&vm.strings);
  initTable(&vm.globals);
  resetStack();
}

/**
 * @brief Set the global in the VM's global table without caring for
 * redefinition.
 *
 * @param name ObjString* the name of the global
 * @param value Value the value to set it to
 * @return bool true if the variable existed in the table previously, false
 * otherwise
 */
static bool setGlobal(ObjString* name, Value value) {
  return !tableSet(&vm.globals, name, value);
}

/**
 * @brief Adds a global to the VM's global table. Reports a runtime error if the
 * global is already defined.
 *
 * @param name ObjString* the name of the global
 * @param value Value the value to set it to
 */
static void addGlobal(ObjString* name, Value value) {
  if (setGlobal(name, value)) {
    runtimeError("Global variable '%s' already defined.", name->chars);
  }
}

/**
 * @brief Runs the chunk stored in the VM.
 *
 * @return InterpretResult The result of the execution.
 */
static InterpretResult run() {
  CallFrame frame = vm.callStack[vm.callStackSize - 1];
#define READ_BYTE() (*frame.ip++)
#define READ_SHORT() (frame.ip += 2, (frame.ip[-2] << 8) | frame.ip[-1])
#define READ_CONSTANT() (frame.chunk->constants.data[READ_BYTE()])
#define READ_STRING() ((ObjString*)TO_OBJECT(READ_CONSTANT()))
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
#ifdef DEBUG_PRINT_OPCODES
        disassembleInstruction(frame.chunk, frame.ip - traveled - frame.chunk->code);
#endif
        return INTERPRET_OK;
      }
      case OP_NULL: {
        push(NULL_VAL);
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
        disassembleInstruction(frame.chunk,
                               (int)(frame.ip - traveled - frame.chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        frame.ip += offset;
        continue;
      }
      case OP_LOOP: {
#ifdef DEBUG_TRACE_EXEC
        disassembleInstruction(frame.chunk,
                               (int)(frame.ip - traveled - frame.chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        if (TO_BOOL(peek(0))) {
          frame.ip -= offset;
          continue;
        }
        break;
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
        disassembleInstruction(frame.chunk,
                               (int)(frame.ip - traveled - frame.chunk->code));
#endif
        uint16_t offset = READ_SHORT();
        Value condition = peek(0);
        if (!TO_BOOL(condition)) {
          frame.ip += offset;
        }
        continue;
      }
        // BINARY OPERATIONS
        ARITHMETIC_OPS_NUM(ADD, +)
        ARITHMETIC_OPS_NUM(SUB, -)
        ARITHMETIC_OPS_NUM(MUL, *)
        ARITHMETIC_OPS_NUM(DIV, /)
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
      case OP_CONSTANT_DOUBLE: {
        Value constant = READ_CONSTANT();
        push(constant);
        traveled++;
        break;
      }
      // Variables
      case OP_GLOBAL_GET: {
        ObjString* name = READ_STRING();
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
        ObjString* name = READ_STRING();
        Value value = pop();
        setGlobal(name, value);
        traveled++;
        break;
      }
      case OP_GLOBAL_DEFINE: {
        ObjString* name = READ_STRING();
        Value value = pop();
        addGlobal(name, value);
        traveled++;
        break;
      }
      case OP_LOCAL_GET: {
        uint8_t slot = READ_BYTE();
        push(frame.slot[slot]);
        traveled++;
        break;
      }
      case OP_LOCAL_SET: {
        uint8_t slot = READ_BYTE();
        frame.slot[slot] = pop();
        traveled++;
        break;
      }
      case OP_PRINT: {
        Value value = pop();
        printValue(value);
        printf("\n");
        break;
      }
    }
    // I like to see the stack after the operation happens
#ifdef DEBUG_TRACE_EXEC
    disassembleInstruction(frame.chunk, (int)(frame.ip - traveled - frame.chunk->code));
    printf("        ");
    for (Value* slot = vm.stack.data; slot < vm.stackTop; slot++) {
      printf("[");
      printValue(*slot);
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
  Chunk chunk;
  initChunk(&chunk);
  initVM();

  if (!compile(source, &chunk)) {
    return INTERPRET_COMPILE_ERROR;
  }
  CallFrame callFrame;
  callFrame.chunk = &chunk;
  callFrame.ip = chunk.code;
  callFrame.slot = vm.stackTop;
  vm.callStack = &callFrame;
  vm.callStackSize += 1;

  InterpretResult result = run();

  freeChunk(&chunk);
  freeVM();

  return result;
}

/**
 * @brief Pushes value onto the stack and increments stackTop.
 *
 * @param value Value the value to push.
 */
void push(Value value) {
  uint8_t dist = (vm.stackTop - vm.stack.data);
  INSERT_DYNAMIC_ARRAY_AT(Value, vm.stack, dist, value);
  vm.stackTop = vm.stack.data + dist + 1;
}

/**
 * @brief Pops the top value off of the stack and returns it.
 *
 * @return Value the value popped off of the stack.
 */
Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

/**
 * @brief Peeks at the value {distance} positions from the top of the stack.
 *
 * @param distance int The distance from the top of the stack to peek at.
 * @return Value The value at the given distance from the top of the stack.
 */
Value peek(int distance) {
  return vm.stack.data[vm.stackTop - vm.stack.data - distance - 1];
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