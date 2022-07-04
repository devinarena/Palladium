
/**
 * @file compiler.c
 * @author Devin Arena
 * @brief Compiles tokens generated by the scanner into opcodes for the VM.
 * @since 6/23/2022
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "dynamic_array.h"
#include "scanner.h"
#include "table.h"
#ifdef DEBUG_PRINT_OPCODES
#include "disassembler.h"
#endif

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
  DYNAMIC_ARRAY(ValueType) typeStack;
  ValueType* typeStackTop;
  Table globalTypes;
} Parser;

typedef struct {
  Token name;
  int depth;
  ValueType valueType;
} Local;

typedef struct {
  Chunk* current;
  int scopeDepth;
  DYNAMIC_ARRAY(Local) locals;
} Compiler;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // . ()
  PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool canAssign);

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

Parser parser;
Compiler* compiler;

/**
 * @brief Initializes the compiler by zeroing out its memory.
 *
 * @param compiler Compiler* a pointer to the compiler to initialize.
 */
static void initCompiler(Compiler* compiler, Chunk* chunk) {
  compiler->current = chunk;
  compiler->scopeDepth = 0;
  INIT_DYNAMIC_ARRAY(Local, compiler->locals);
}

/**
 * @brief Shows an error message for the specified token and line number.
 *
 * @param token Token* the token to show the error message for.
 * @param message const char* the error message to show.
 */
static void errorAtToken(Token* token, const char* message) {
  if (parser.panicMode)
    return;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

/**
 * @brief Prints an error message at the current token and puts the parser in
 * panic mode.
 *
 * @param message const char* the error message to print.
 */
static void parseError(const char* message) {
  errorAtToken(&parser.previous, message);
  parser.panicMode = true;
}

/**
 * @brief Advances the parser to the next token. If an error occurs, the parser
 * will stop parsing and emit a parse error.
 *
 */
static void advance() {
  parser.previous = parser.current;

  while (true) {
    parser.current = scanToken();

    if (parser.current.type != TOKEN_ERROR)
      break;

    parseError(parser.current.start);
  }
}

/**
 * @brief Checks for the specified token. If it is found, the parser will
 * advance to the next token. If not, the parser will emit a parse error.
 *
 * @param type TokenType the type of token to check for.
 * @param message const char* the error message to show.
 */
static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  parseError(message);
}

/**
 * @brief Checks the current token for the specified type. Returns true if it
 * matches, false otherwise.
 *
 * @param type TokenType the type to check for.
 * @return bool true if the current token matches the specified type, false
 * otherwise
 */
bool check(TokenType type) {
  return parser.current.type == type;
}

/**
 * @brief Consumes the current token if it matches the specfied type. Returns if
 * it matches.
 *
 * @param type TokenType the type of token to check for.
 * @return bool true if the token matches, false otherwise.
 */
bool match(TokenType type) {
  if (check(type)) {
    advance();
    return true;
  }

  return false;
}

/**
 * @brief Handles the parser being in panic mode by skipping all tokens until
 * the start of a declaration.
 *
 */
static void synchronize() {
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    // if the last token was a semicolon, we also consider it to be the end of a
    // statement.
    if (parser.previous.type == TOKEN_SEMICOLON)
      return;

    // these tokens designate the start of a statement and therefore
    // (hopefully) the end of panic mode
    switch (parser.current.type) {
      case TOKEN_VOID:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:
        break;
    }

    advance();
  }
}

/**
 * @brief Pushes a type onto the type stack for type checking.
 *
 * @param type ValueType the type to push.
 */
static void pushType(ValueType type) {
  uint8_t dist = (parser.typeStackTop - parser.typeStack.data);
  INSERT_DYNAMIC_ARRAY_AT(ValueType, parser.typeStack, dist, type);
  parser.typeStackTop = parser.typeStack.data + dist + 1;
}

/**
 * @brief Pops a type from the type stack for type checking.
 *
 * @return ValueType the type popped.
 */
static ValueType popType() {
  if (parser.typeStackTop == parser.typeStack.data)
    return VALUE_NULL;
  parser.typeStackTop--;
  return *parser.typeStackTop;
}

/**
 * @brief Peeks at the top of the type stack for type checking.
 *
 * @return ValueType the type at the top of the stack.
 */
static ValueType peekType() {
  if (parser.typeStackTop == parser.typeStack.data)
    return VALUE_NULL;
  return *(parser.typeStackTop - 1);
}

/**
 * @brief Swaps the top two types on the type stack for type checking.
 */
static void swapTypes() {
  ValueType a = popType();
  ValueType b = popType();
  pushType(a);
  pushType(b);
}

/**
 * @brief Helper for emitting an opcode. Writes the opcode or extra data to the
 * current chunk.
 *
 * @param byte byte the opcode to write
 */
static void emitByte(uint8_t byte) {
  writeToChunk(compiler->current, byte, parser.previous.line);
}

/**
 * @brief Helper for emitting two bytes.
 *
 * @param byte1 uint8_t the first byte to emit
 * @param byte2 uint8_t the second byte to emit
 */
static void emitBytes(uint8_t byte1, uint8_t byte2) {
  emitByte(byte1);
  emitByte(byte2);
}

/**
 * @brief Helper for emitting a jump (jump opcode).
 *
 * @param byte uint8_t the jump opcode to emit
 */
static int emitJump(uint8_t byte) {
  emitByte(byte);
  emitBytes(0xFF, 0xFF);
  return compiler->current->count - 3;
}

/**
 * @brief Emits a loop to and the offset to loop back to.
 *
 * @param instruction int the instruction to loop back to.
 */
static void emitLoop(int instruction) {
  int offset = compiler->current->count - instruction;
  emitByte(OP_LOOP);
  emitBytes((offset >> 8) & 0xFF, offset & 0xFF);
}

/**
 * @brief Patches a jump by setting the bytes at the specified offset to the
 * correct jump location (current instruction count).
 *
 * @param offset
 */
static void patchJump(int offset) {
  int jump = compiler->current->count - offset - 3;
  if (jump > UINT16_MAX) {
    parseError("Too much code to jump over.");
    return;
  }
  compiler->current->code[offset + 1] = (jump >> 8) & 0xFF;
  compiler->current->code[offset + 2] = jump & 0xFF;
}

/**
 * @brief Helper for emitting a return opcode.
 */
static void emitReturn() {
  emitByte(OP_RETURN);
}

/**
 * @brief Emits a constant with the specified opcode (type specific).
 *
 * @param opcode the opcode to emit
 * @param constant the constant to emit
 */
static void emitConstant(int opcode, Value constant) {
  uint8_t index = (uint8_t)addConstant(compiler->current, constant);
  emitBytes(opcode, index);
}

/**
 * @brief Helper for comparing two tokens.
 *
 * @param a ValueType the first ValueType to compare
 * @param b ValueType the second ValueType to compare
 * @param useNum if we should consider ints and floats to be equal
 *
 * @return bool true if the ValueTypes are equal, false otherwise
 */
bool typesEqual(ValueType a, ValueType b, bool useNum) {
  if (useNum) {
    if (IS_NUMBER_TYPE(a) && IS_NUMBER_TYPE(b)) {
      return true;
    }
  }

  return a == b;
}

/**
 * @brief Adds an identifiers name to the list of constants.
 *
 * @param name Token* name of the identifier
 * @return uint8_t index of the constant in the constant list
 */
static uint8_t identifierConstant(Token* name) {
  return addConstant(compiler->current,
                     FROM_OBJECT(copyString(name->start, name->length)));
}

/**
 * @brief Generates and returns the index of the constant pointing to a string
 * with the given identifier name.
 *
 * @param message char* the error message if no identifier is provided
 * @return uint8_t the index of the constant pointing to the string with the
 * identifier name
 */
static uint8_t parseVariable(const char* message) {
  consume(TOKEN_IDENTIFIER, message);

  return identifierConstant(&parser.previous);
}

/**
 * @brief Resolves a local by searching the current scope from the end (to get
 * most recently declared).
 *
 * @param name Token* name of the local to resolve
 * @return int index of the local in the local list
 */
static int resolveLocal(Token* name) {
  for (int i = compiler->locals.count - 1; i >= 0; i--) {
    Local local = compiler->locals.data[i];
    if (local.name.length == name->length &&
        memcmp(local.name.start, name->start, name->length) == 0) {
      if (local.depth == -1) {
        parseError("Cannot read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

/**
 * @brief Adds a local to the current compilers locals list.
 *
 * @param name Token name of the local
 * @param type ValueType type of the local
 */
static void addLocal(Token name, ValueType type) {
  if (compiler->scopeDepth == 0) {
    parseError("Cannot declare local variables at the top level.");
    return;
  }
  for (int i = compiler->locals.count - 1; i >= 0; i--) {
    Local local = compiler->locals.data[i];
    if (local.depth != -1 && local.depth < compiler->scopeDepth) {
      break;
    }
    if (local.name.length == name.length &&
        memcmp(local.name.start, name.start, name.length) == 0) {
      parseError("Cannot declare two variables with the same name.");
      return;
    }
  }
  Local local;
  local.name = name;
  local.valueType = type;
  local.depth = -1;
  INSERT_DYNAMIC_ARRAY(Local, compiler->locals, local);
}

/**
 * @brief Increments scope depth.
 */
static void pushScope() {
  compiler->scopeDepth++;
}

/**
 * @brief Decrements scope depth.
 */
static void popScope() {
  compiler->scopeDepth--;

  // remove all locals from the current scope
  for (int i = compiler->locals.count - 1; i >= 0; i--) {
    Local local = compiler->locals.data[i];
    if (local.depth > compiler->scopeDepth) {
      emitByte(OP_POP);
      compiler->locals.count--;
    } else
      break;
  }
}

/**
 * @brief Handles shutting down the compiler. Emits a return opcode.
 */
static void endCompiler() {
  emitReturn();
  FREE_DYNAMIC_ARRAY(Local, compiler->locals);
}

// RECURSIVE DESCENT

static void parsePrecedence(Precedence prec);
static ParseRule* getRule(TokenType type);
static void expression();
static void statement();
static void declaration();

/**
 * @brief Adds the parsed integer to the chunk's constants and outputs
 * OP_CONSTANT_INT with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void integer(bool canAssign) {
  int value = atoi(parser.previous.start);
  emitConstant(OP_CONSTANT_INT, FROM_INTEGER(value));
  pushType(VALUE_INTEGER);
}

/**
 * @brief Adds the parsed float to the chunk's constants and outputs
 * OP_CONSTANT_FLOAT with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void double_(bool canAssign) {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(OP_CONSTANT_DOUBLE, FROM_DOUBLE(value));
  pushType(VALUE_DOUBLE);
}

/**
 * @brief Adds the parsed bool to the chunk's constants and outputs
 * OP_CONSTANT_BOOL with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void literal(bool canAssign) {
  if (parser.previous.type == TOKEN_NULL) {
    emitByte(OP_NULL);
    pushType(VALUE_NULL);
    return;
  }
  emitConstant(OP_CONSTANT_BOOL, FROM_BOOL(parser.previous.type == TOKEN_TRUE));
  pushType(VALUE_BOOL);
}

/**
 * @brief Adds the parsed string to the chunk's constants and outputs
 * OP_CONSTANT_CHARACTER with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void char_(bool canAssign) {
  char c = *(parser.previous.start + 1);
  emitConstant(OP_CONSTANT_CHARACTER, FROM_CHARACTER(c));
  pushType(VALUE_CHARACTER);
}

/**
 * @brief Adds the parsed string to the chunk's constants and outputs
 * OP_CONSTANT_STRING with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void string(bool canAssign) {
  emitConstant(OP_CONSTANT_STRING,
               FROM_OBJECT(copyString(parser.previous.start + 1,
                                      parser.previous.length - 2)));
  pushType(VALUE_OBJECT);
}

/**
 * @brief Handles grouping expressions. Just recurses into the expression first.
 *
 * @param canAssign bool whether or not the expression can be assigned to
 */
static void grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after grouping.");
}

/**
 * @brief Helper for generating opcodes for unary instructions e.g. negate.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void unary(bool canAssign) {
  ValueType previous = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  ValueType current = popType();

  switch (previous) {
    case TOKEN_MINUS:
      switch (current) {
        case VALUE_INTEGER:
          emitByte(OP_NEGATE_INT);
          pushType(VALUE_INTEGER);
          break;
        case VALUE_DOUBLE:
          emitByte(OP_NEGATE_DOUBLE);
          pushType(VALUE_DOUBLE);
          break;
        default:
          parseError("Cannot negate non-numeric value.");
          break;
      }
      break;
    case TOKEN_BANG:
      pushType(VALUE_BOOL);
      switch (current) {
        case VALUE_INTEGER:
          emitByte(OP_NOT_NUMBER);
          break;
        case VALUE_DOUBLE:
          emitByte(OP_NOT_NUMBER);
          break;
        case VALUE_BOOL:
          emitByte(OP_NOT_BOOL);
          break;
        default:
          parseError("Unreachable.");
          break;
      }
      break;
    case TOKEN_REFERENCE:
      emitByte(OP_REFERENCE);
      pushType(current);
      pushType(VALUE_POINTER);
      break;
    case TOKEN_STAR:
      emitByte(OP_DEREFERENCE);
      break;
    default:
      parseError("Unary operator expected");
      break;
  }
}

#define BINARY_OPERATOR_CASE_NUM_RESULT(operator, instruction)        \
  case (operator): {                                                  \
    if (before == VALUE_INTEGER && after == VALUE_INTEGER) {          \
      emitByte(OP_##instruction##_INT);                               \
      pushType(VALUE_INTEGER);                                        \
    } else if (before == VALUE_DOUBLE && after == VALUE_DOUBLE) {     \
      emitByte(OP_##instruction##_DOUBLE);                            \
      pushType(VALUE_DOUBLE);                                         \
    } else if (before == VALUE_INTEGER && after == VALUE_DOUBLE) {    \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                        \
      emitByte(OP_##instruction##_DOUBLE);                            \
      pushType(VALUE_DOUBLE);                                         \
    } else if (before == VALUE_DOUBLE && after == VALUE_INTEGER) {    \
      emitByte(OP_SWAP);                                              \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                        \
      emitByte(OP_SWAP);                                              \
      emitByte(OP_##instruction##_DOUBLE);                            \
      pushType(VALUE_DOUBLE);                                         \
    } else if ((before == VALUE_POINTER && after == VALUE_INTEGER) || \
               (before == VALUE_INTEGER && after == VALUE_POINTER)) { \
    } else {                                                          \
      parseError("Binary operator invalid for given values.");        \
    }                                                                 \
    break;                                                            \
  }

#define BINARY_OPERATOR_CASE_NUM_RESULT_POINTERS(operator, instruction) \
  case (operator): {                                                    \
    if (before == VALUE_INTEGER && after == VALUE_INTEGER) {            \
      emitByte(OP_##instruction##_INT);                                 \
      pushType(VALUE_INTEGER);                                          \
    } else if (before == VALUE_DOUBLE && after == VALUE_DOUBLE) {       \
      emitByte(OP_##instruction##_DOUBLE);                              \
      pushType(VALUE_DOUBLE);                                           \
    } else if (before == VALUE_INTEGER && after == VALUE_DOUBLE) {      \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                          \
      emitByte(OP_##instruction##_DOUBLE);                              \
      pushType(VALUE_DOUBLE);                                           \
    } else if (before == VALUE_DOUBLE && after == VALUE_INTEGER) {      \
      emitByte(OP_SWAP);                                                \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                          \
      emitByte(OP_SWAP);                                                \
      emitByte(OP_##instruction##_DOUBLE);                              \
      pushType(VALUE_DOUBLE);                                           \
    } else if (before == VALUE_POINTER && after == VALUE_INTEGER) {     \
      emitByte(OP_##instruction##_POINTER);                             \
    } else if (before == VALUE_INTEGER && after == VALUE_POINTER) {     \
      emitBytes(OP_SWAP, OP_##instruction##_POINTER);                                        \
    } else {                                                            \
      parseError("Binary operator invalid for given values.");          \
    }                                                                   \
    break;                                                              \
  }

#define BINARY_OPERATOR_CASE_BOOL_RESULT(operator, instruction)    \
  case (operator): {                                               \
    if (before == VALUE_INTEGER && after == VALUE_INTEGER) {       \
      emitByte(OP_##instruction##_INT);                            \
    } else if (before == VALUE_DOUBLE && after == VALUE_DOUBLE) {  \
      emitByte(OP_##instruction##_DOUBLE);                         \
    } else if (before == VALUE_INTEGER && after == VALUE_DOUBLE) { \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                     \
      emitByte(OP_##instruction##_DOUBLE);                         \
    } else if (before == VALUE_DOUBLE && after == VALUE_INTEGER) { \
      emitByte(OP_SWAP);                                           \
      emitByte(OP_ARITHMETIC_CAST_INT_DOUBLE);                     \
      emitByte(OP_SWAP);                                           \
      emitByte(OP_##instruction##_DOUBLE);                         \
    } else {                                                       \
      parseError("Binary operator invalid for given values.");     \
      return;                                                      \
    }                                                              \
    pushType(VALUE_BOOL);                                          \
    break;                                                         \
  }

/**
 * @brief Handles binary operators, such as addition/subtraction, equality, etc.
 *
 * @param canAssign bool whether or not the expression can be assigned to
 */
static void binary(bool canAssign) {
  TokenType operator= parser.previous.type;
  ParseRule* rule = getRule(operator);

  parsePrecedence((Precedence)(rule->precedence + 1));

  ValueType before = popType();
  ValueType after = popType();

  switch (operator) {
    BINARY_OPERATOR_CASE_NUM_RESULT_POINTERS(TOKEN_PLUS, ADD);
    BINARY_OPERATOR_CASE_NUM_RESULT_POINTERS(TOKEN_MINUS, SUB);
    BINARY_OPERATOR_CASE_NUM_RESULT(TOKEN_STAR, MUL);
    BINARY_OPERATOR_CASE_NUM_RESULT(TOKEN_SLASH, DIV);
    BINARY_OPERATOR_CASE_BOOL_RESULT(TOKEN_GREATER, GREATER);
    BINARY_OPERATOR_CASE_BOOL_RESULT(TOKEN_GREATER_EQUAL, GREATER_EQUAL);
    BINARY_OPERATOR_CASE_BOOL_RESULT(TOKEN_LESS, LESS);
    BINARY_OPERATOR_CASE_BOOL_RESULT(TOKEN_LESS_EQUAL, LESS_EQUAL);
    case TOKEN_EQUAL_EQUAL: {
      if (typesEqual(before, after, true)) {
        emitByte(OP_EQUALITY);
        pushType(VALUE_BOOL);
      } else {
        parseError("Cannot compare values of different type.");
      }
      break;
    }
    case TOKEN_BANG_EQUAL: {
      if (typesEqual(before, after, true)) {
        emitBytes(OP_EQUALITY, OP_NOT_BOOL);
        pushType(VALUE_BOOL);
      } else {
        parseError("Cannot compare values of different type.");
      }
      break;
    }
  }
}

/**
 * @brief Generates the proper opcodes for getting or setting a variable.
 *
 * @param name Token* the name of the variable
 * @param canAssign bool whether or not the variable can be assigned to
 */
static void namedVariable(Token* name, bool canAssign) {
  int arg = resolveLocal(name);
  if (arg != -1) {
    Local local = compiler->locals.data[arg];
    if (canAssign && match(TOKEN_EQUAL)) {
      expression();
      ValueType type = popType();
      if (type != local.valueType) {
        parseError("Cannot assign value of different type.");
      }
      emitBytes(OP_LOCAL_SET, (uint8_t)arg);
      emitByte(OP_POP);
      local.depth = compiler->scopeDepth;
    } else {
      emitBytes(OP_LOCAL_GET, (uint8_t)arg);
      pushType(local.valueType);
    }
  } else {
    arg = identifierConstant(name);
    if (canAssign && match(TOKEN_EQUAL)) {
      expression();
      ValueType type = popType();
      Value value;
      if (!tableGet(
              &parser.globalTypes,
              (ObjString*)TO_OBJECT(compiler->current->constants.data[arg]),
              &value)) {
        parseError("Cannot assign to undeclared variable.");
      }
      if (type != value.type) {
        parseError("Cannot assign value of different type.");
      }
      emitBytes(OP_GLOBAL_SET, arg);
    } else {
      emitBytes(OP_GLOBAL_GET, arg);
      Value value;
      if (!tableGet(
              &parser.globalTypes,
              (ObjString*)TO_OBJECT(compiler->current->constants.data[arg]),
              &value)) {
        parseError("Referenced variable is undefined.");
        return;
      }
      pushType(value.type);
    }
  }
}

/**
 * @brief Descent case for variables, calls helper namedVariable.
 *
 * @param canAssign bool whether or not the variable can be assigned to
 */
static void variable(bool canAssign) {
  namedVariable(&parser.previous, canAssign);
}

static void and (bool canAssign) {
  ValueType a = popType();
  if (a != VALUE_BOOL) {
    parseError("And operator must be used with boolean operands.");
  }
  int jump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  parsePrecedence(PREC_AND);
  ValueType b = popType();
  if (b != VALUE_BOOL) {
    parseError("And operator must be used with boolean operands.");
  }
  patchJump(jump);
  pushType(VALUE_BOOL);
}

static void or (bool canAssign) {
  ValueType a = popType();
  if (a != VALUE_BOOL) {
    parseError("Or operator must be used with boolean operands.");
  }
  int jump = emitJump(OP_JUMP_IF_TRUE);
  emitByte(OP_POP);
  parsePrecedence(PREC_OR);
  ValueType b = popType();
  if (b != VALUE_BOOL) {
    parseError("Or operator must be used with boolean operands.");
  }
  patchJump(jump);
  pushType(VALUE_BOOL);
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN] = {grouping, NULL, PREC_CALL},
    [TOKEN_RIGHT_PAREN] = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE] = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT] = {NULL, NULL, PREC_CALL},
    [TOKEN_SEMICOLON] = {NULL, NULL, PREC_NONE},
    [TOKEN_REFERENCE] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG] = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL] = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {variable, NULL, PREC_NONE},
    [TOKEN_STRING] = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER_INTEGER] = {integer, NULL, PREC_NONE},
    [TOKEN_NUMBER_FLOATING] = {double_, NULL, PREC_NONE},
    // char is the keyword, character is the literal
    [TOKEN_CHARACTER] = {char_, NULL, PREC_NONE},
    [TOKEN_TRUE] = {literal, NULL, PREC_NONE},
    [TOKEN_FALSE] = {literal, NULL, PREC_NONE},
    [TOKEN_MINUS] = {unary, binary, PREC_TERM},
    [TOKEN_PLUS] = {NULL, binary, PREC_TERM},
    [TOKEN_SLASH] = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR] = {unary, binary, PREC_FACTOR},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_AND] = {NULL, and, PREC_AND},
    [TOKEN_OR] = {NULL, or, PREC_OR},
    [TOKEN_NULL] = {literal, NULL, PREC_NONE},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_INT] = {NULL, NULL, PREC_NONE},
    [TOKEN_DOUBLE] = {NULL, NULL, PREC_NONE},
    [TOKEN_BOOL] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR] = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF] = {NULL, NULL, PREC_NONE},
};

/**
 * @brief Get the Rule for the given token.
 *
 * @param type TokenType the token type to get the rule for
 * @return ParseRule* the rule for the given token
 */
ParseRule* getRule(TokenType type) {
  return &rules[type];
}

/**
 * @brief Pratt parser implementation. Parses an expression, handling order of
 * precedence properly.
 *
 * @param prec the precedence to parse up to
 */
static void parsePrecedence(Precedence prec) {
  advance();
  ParseFn prefix = getRule(parser.previous.type)->prefix;
  // the first rule we hit will always be a prefix rule.
  if (prefix == NULL) {
    parseError("Expected expression.");
    return;
  }

  bool canAssign = prec <= PREC_ASSIGNMENT;

  prefix(canAssign);

  while (prec <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infix = getRule(parser.previous.type)->infix;
    infix(canAssign);
  }

  if (!canAssign && match(TOKEN_EQUAL)) {
    parseError("Invalid assignment target.");
  }

#ifdef DEBUG_TRACE_EXEC
  printf("Type Stack: ");
  for (ValueType* slot = parser.typeStack.data; slot < parser.typeStackTop;
       slot++) {
    ValueType t = *slot;
    printf("[%d]", t);
  }
  printf("\n");
#endif
}

/**
 * @brief Descent case for an expression, parses with assignment precedence.
 */
static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

/**
 * @brief Descent case for print statements. Prints the top of the stack.
 */
static void printStatement() {
  expression();
  popType();
  consume(TOKEN_SEMICOLON, "Expected ';' after print statement.");
  emitByte(OP_PRINT);
}

/**
 * @brief Descent case for parsing blocks. Recursively calls declaration while
 * not at the end of the block.
 */
static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expected '}' after block.");
}

/**
 * @brief Descent case for if statement, parses the condition and then the if
 * and else blocks.
 */
static void ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Expected '(' after if.");
  expression();
  if (popType() != VALUE_BOOL) {
    parseError("Expected boolean condition.");
  }
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after if condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();

  int elseJump = emitJump(OP_JUMP);

  patchJump(thenJump);
  emitByte(OP_POP);

  if (match(TOKEN_ELSE)) {
    statement();
  }

  patchJump(elseJump);
}

/**
 * @brief Descent case for expression statements (expressions terminated with a
 * ';')
 */
static void expressionStatement() {
  expression();
  popType();
  consume(TOKEN_SEMICOLON, "Expect ';' following expression.");
}

/**
 * @brief Descent case for while statements (a condition and loop body)
 *
 */
static void whileStatement() {
  int loopStart = compiler->current->count - 3;
  consume(TOKEN_LEFT_PAREN, "Expected '(' after while.");
  expression();
  if (popType() != VALUE_BOOL) {
    parseError("Expected boolean condition.");
  }
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after while condition.");

  int exitJump = emitJump(OP_JUMP_IF_FALSE);
  emitByte(OP_POP);
  statement();
  emitLoop(loopStart);
  patchJump(exitJump);
  emitByte(OP_POP);
}

/**
 * @brief Descent case for for statements (a condition, an expression, loop
 * body, and a post-loop expression).
 */
static void forStatement() {
  pushScope();
  consume(TOKEN_LEFT_PAREN, "Expected '(' after for.");

  if (!match(TOKEN_SEMICOLON)) {
    declaration();
  }

  // need two loops, loop back to the post expression after
  // then from the post expression loop back to the condition
  int loopStart = compiler->current->count - 3;
  int exitJump = -1;

  if (!match(TOKEN_SEMICOLON)) {
    expression();
    popType();
    consume(TOKEN_SEMICOLON, "Expected ';' after loop condition.");
    exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
  }

  if (!match(TOKEN_RIGHT_PAREN)) {
    // jump over the post expression (will jump back later)
    int bodyJump = emitJump(OP_JUMP);
    int incrementStart = compiler->current->count - 3;
    expression();
    popType();
    consume(TOKEN_RIGHT_PAREN, "Expected ')' after for loop.");
    // loop back to the condition
    emitLoop(loopStart);
    loopStart = incrementStart;
    patchJump(bodyJump);
  }

  statement();
  // loop back to the post expression
  emitLoop(loopStart);

  if (exitJump != -1) {
    patchJump(exitJump);
    emitByte(OP_POP);
  }

  popScope();
}

/**
 * @brief Descent case for parsing statements, e.g. print, if, while, etc.
 */
static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_WHILE)) {
    whileStatement();
  } else if (match(TOKEN_FOR)) {
    forStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    pushScope();
    block();
    popScope();
  } else {
    expressionStatement();
  }
}

#define DECLARATION(T, value)                                                  \
  static void declaration##T() {                                               \
    uint8_t index = parseVariable("Expected variable name.");                  \
    uint8_t op = OP_GLOBAL_DEFINE;                                             \
    Token name = parser.previous;                                              \
    if (match(TOKEN_EQUAL)) {                                                  \
      expression();                                                            \
      if (popType() != value) {                                                \
        parseError("Initializer does not match declared type.");               \
      }                                                                        \
    } else {                                                                   \
      emitByte(OP_NULL);                                                       \
    }                                                                          \
    if (compiler->scopeDepth == 0) {                                           \
      if (!tableSet(                                                           \
              &parser.globalTypes,                                             \
              (ObjString*)TO_OBJECT(compiler->current->constants.data[index]), \
              (Value){.type = value}))                                         \
        parseError("Global variable already defined.");                        \
    } else {                                                                   \
      addLocal(name, value);                                                   \
      op = OP_LOCAL_SET;                                                       \
    }                                                                          \
    consume(TOKEN_SEMICOLON, "Expected ';' after variable declaration.");      \
    emitBytes(op, index);                                                      \
    if (op == OP_LOCAL_SET) {                                                  \
      compiler->locals.data[compiler->locals.count - 1].depth =                \
          compiler->scopeDepth;                                                \
    }                                                                          \
  }

/**
 * @brief Descent case for integer declaration, int followed by an identifier
 * and initializer.
 */
DECLARATION(Integer, VALUE_INTEGER)

/**
 * @brief Descent case for double declaration, double followed by an identifier
 * and initializer.
 */
DECLARATION(Double, VALUE_DOUBLE);

/**
 * @brief Descent case for boolean declaration, bool followed by an identifier
 * and initializer.
 */
DECLARATION(Boolean, VALUE_BOOL);

/**
 * @brief Descent case for character declaration, char followed by an identifier
 * and initializer.
 */
DECLARATION(Character, VALUE_CHARACTER);

/**
 * @brief Descent case for parsing declarations, e.g. var, function, etc.
 */
static void declaration() {
  if (match(TOKEN_INT)) {
    declarationInteger();
  } else if (match(TOKEN_DOUBLE)) {
    declarationDouble();
  } else if (match(TOKEN_BOOL)) {
    declarationBoolean();
  } else if (match(TOKEN_CHAR)) {
    declarationCharacter();
  } else {
    statement();
  }
}

/**
 * @brief Compiles the source into opcodes for the VM to process.
 *
 * @param source const char* the source string to compile.
 * @param chunk Chunk* the chunk to compile opcodes into.
 * @return bool true if the compilation was successful, false otherwise.
 */
bool compile(const char* source, Chunk* chunk) {
  initScanner(source);
  Compiler comp;
  initCompiler(&comp, chunk);
  compiler = &comp;
  INIT_DYNAMIC_ARRAY(ValueType, parser.typeStack);
  initTable(&parser.globalTypes);

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (parser.current.type != TOKEN_EOF) {
    declaration();
  }

  endCompiler();

  FREE_DYNAMIC_ARRAY(ValueType, parser.typeStack);
  freeTable(&parser.globalTypes);

#ifdef DEBUG_PRINT_OPCODES
  if (!parser.hadError) {
    disassembleChunk(chunk, "code");
  }
#endif

  return !parser.hadError;
}