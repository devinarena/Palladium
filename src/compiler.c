
/**
 * @file compiler.c
 * @author Devin Arena
 * @brief Compiles tokens generated by the scanner into opcodes for the VM.
 * @since 6/23/2022
 **/

#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "dynamic_array.h"
#include "scanner.h"
#ifdef DEBUG_PRINT_OPCODES
#include "disassembler.h"
#endif

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
  DYNAMIC_ARRAY(TokenType) typeStack;
  TokenType* typeStackTop;
} Parser;

typedef struct {
  Chunk* current;
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
  errorAtToken(&parser.current, message);
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
 * @brief Consumes the current token if it matches the specfied type. Returns if
 * it matches.
 *
 * @param type TokenType the type of token to check for.
 * @return bool true if the token matches, false otherwise.
 */
bool match(TokenType type) {
  if (parser.current.type == type) {
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
 * @param type TokenType the type to push.
 */
static void pushType(TokenType type) {
  uint8_t dist = (parser.typeStackTop - parser.typeStack.data);
  INSERT_DYNAMIC_ARRAY_AT(TokenType, parser.typeStack, parser.typeStack.count,
                          type);
  parser.typeStackTop = parser.typeStack.data + dist + 1;
}

/**
 * @brief Pops a type from the type stack for type checking.
 *
 * @return TokenType the type popped.
 */
static TokenType popType() {
  parser.typeStackTop--;
  return *parser.typeStackTop;
}

/**
 * @brief Peeks at the top of the type stack for type checking.
 *
 * @return TokenType the type at the top of the stack.
 */
static TokenType peekType() {
  return *(parser.typeStackTop - 1);
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
 * @param a TokenType the first TokenType to compare
 * @param b TokenType the second TokenType to compare
 * @param useNum if we should consider ints and floats to be equal
 *
 * @return bool true if the TokenTypes are equal, false otherwise
 */
bool typesEqual(TokenType a, TokenType b, bool useNum) {
  if (useNum) {
    if ((a == TOKEN_NUMBER_INTEGER || a == TOKEN_NUMBER_FLOATING) &&
        (b == TOKEN_NUMBER_INTEGER || b == TOKEN_NUMBER_FLOATING)) {
      return true;
    }
  }

  return a == b;
}

/**
 * @brief Handles shutting down the compiler. Emits a return opcode.
 */
static void endCompiler() {
  emitReturn();
}

// RECURSIVE DESCENT

static void parsePrecedence(Precedence prec);
static ParseRule* getRule(TokenType type);
static void expression();
static void statement();

/**
 * @brief Adds the parsed integer to the chunk's constants and outputs
 * OP_CONSTANT_INT with its index.
 *
 * @param canAssign bool whether or not the constant can be assigned to (never)
 */
static void integer(bool canAssign) {
  int value = atoi(parser.previous.start);
  emitConstant(OP_CONSTANT_INT, FROM_INTEGER(value));
  pushType(TOKEN_NUMBER_INTEGER);
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
  pushType(TOKEN_NUMBER_FLOATING);
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
    pushType(TOKEN_NULL);
    return;
  }
  emitConstant(OP_CONSTANT_BOOL, FROM_BOOL(parser.previous.type == TOKEN_TRUE));
  pushType(TOKEN_BOOL);
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
  pushType(TOKEN_STRING);
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
  TokenType previous = parser.previous.type;

  parsePrecedence(PREC_UNARY);

  TokenType current = popType();

  switch (previous) {
    case TOKEN_MINUS:
      switch (current) {
        case TOKEN_NUMBER_INTEGER:
          emitByte(OP_NEGATE_INT);
          pushType(TOKEN_NUMBER_INTEGER);
          break;
        case TOKEN_NUMBER_FLOATING:
          emitByte(OP_NEGATE_DOUBLE);
          pushType(TOKEN_NUMBER_FLOATING);
          break;
        default:
          parseError("Cannot negate non-numeric value.");
          break;
      }
      break;
    case TOKEN_BANG:
      pushType(TOKEN_BOOL);
      printf("%d\n", peekType());
      switch (current) {
        case TOKEN_NUMBER_INTEGER:
          emitByte(OP_NOT_NUMBER);
          break;
        case TOKEN_NUMBER_FLOATING:
          emitByte(OP_NOT_NUMBER);
          break;
        case TOKEN_BOOL:
          emitByte(OP_NOT_BOOL);
          break;
        default:
          parseError("Unreachable.");
          break;
      }
      break;
    case TOKEN_REFERENCE:
      emitByte(OP_REFERENCE);
      pushType(TOKEN_REFERENCE);
      break;
    default:
      parseError("Unary operator expected");
      break;
  }
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

  TokenType before = popType();
  TokenType after = popType();

  switch (operator) {
    case TOKEN_PLUS: {
      if (typesEqual(before, after, false) && before == TOKEN_NUMBER_INTEGER) {
        emitByte(OP_ADD_INT);
        pushType(TOKEN_NUMBER_INTEGER);
      } else if (typesEqual(before, after, false) &&
                 before == TOKEN_NUMBER_FLOATING) {
        emitByte(OP_ADD_DOUBLE);
        pushType(TOKEN_NUMBER_FLOATING);
      } else {
        parseError("Cannot add non-numeric values.");
      }
      break;
    }
    case TOKEN_MINUS: {
      if (typesEqual(before, after, false) && before == TOKEN_NUMBER_INTEGER) {
        emitByte(OP_SUB_INT);
        pushType(TOKEN_NUMBER_INTEGER);
      } else if (typesEqual(before, after, false) &&
                 before == TOKEN_NUMBER_FLOATING) {
        emitByte(OP_SUB_DOUBLE);
        pushType(TOKEN_NUMBER_FLOATING);
      } else {
        parseError("Cannot subtract non-numeric values.");
      }
      break;
    }
    case TOKEN_STAR: {
      if (typesEqual(before, after, false) && before == TOKEN_NUMBER_INTEGER) {
        emitByte(OP_MUL_INT);
        pushType(TOKEN_NUMBER_INTEGER);
      } else if (typesEqual(before, after, false) &&
                 before == TOKEN_NUMBER_FLOATING) {
        emitByte(OP_MUL_DOUBLE);
        pushType(TOKEN_NUMBER_FLOATING);
      } else {
        parseError("Cannot multiply non-numeric values.");
      }
      break;
    }
    case TOKEN_SLASH: {
      if (typesEqual(before, after, false) && before == TOKEN_NUMBER_INTEGER) {
        emitByte(OP_DIV_INT);
        pushType(TOKEN_NUMBER_INTEGER);
      } else if (typesEqual(before, after, false) &&
                 before == TOKEN_NUMBER_FLOATING) {
        emitByte(OP_DIV_DOUBLE);
        pushType(TOKEN_NUMBER_FLOATING);
      } else {
        parseError("Cannot divide non-numeric values.");
      }
      break;
    }
    case TOKEN_EQUAL_EQUAL: {
      if (typesEqual(before, after, true)) {
        emitByte(OP_EQUALITY);
        pushType(TOKEN_BOOL);
      } else {
        parseError("Cannot compare values of different type.");
      }
      break;
    }
    case TOKEN_BANG_EQUAL: {
      if (typesEqual(before, after, true)) {
        emitBytes(OP_EQUALITY, OP_NOT_BOOL);
        pushType(TOKEN_BOOL);
      } else {
        parseError("Cannot compare values of different type.");
      }
      break;
    }
  }
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
    [TOKEN_BANG_EQUAL] = {NULL, NULL, PREC_EQUALITY},
    [TOKEN_EQUAL] = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL] = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER] = {NULL, NULL, PREC_NONE},
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
    [TOKEN_STAR] = {NULL, binary, PREC_FACTOR},
    [TOKEN_AND] = {NULL, NULL, PREC_AND},
    [TOKEN_ELSE] = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR] = {NULL, NULL, PREC_NONE},
    [TOKEN_IF] = {NULL, NULL, PREC_NONE},
    [TOKEN_NULL] = {literal, NULL, PREC_NONE},
    [TOKEN_OR] = {NULL, NULL, PREC_OR},
    [TOKEN_PRINT] = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN] = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE] = {NULL, NULL, PREC_NONE},
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

  prefix(false);

  while (prec <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infix = getRule(parser.previous.type)->infix;
    infix(canAssign);
  }

#ifdef DEBUG_TRACE_EXEC
  printf("Type Stack: ");
  for (TokenType* slot = parser.typeStack.data; slot < parser.typeStackTop;
       slot++) {
    printf("[");
    printf("%d", *slot);
    printf("]");
  }
  printf("\n");
#endif
}

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
 * @brief Descent case for parsing statements, e.g. print, if, while, etc.
 */
static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else {
    parseError("Expected statement.");
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
  INIT_DYNAMIC_ARRAY(TokenType, parser.typeStack);

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (parser.current.type != TOKEN_EOF) {
    statement();
  }

  endCompiler();

  FREE_DYNAMIC_ARRAY(TokenType, parser.typeStack);

#ifdef DEBUG_PRINT_OPCODES
  if (!parser.hadError) {
    disassembleChunk(chunk, "code");
  }
#endif

  return !parser.hadError;
}