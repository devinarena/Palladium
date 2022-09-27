/**
 * @file scanner.h
 * @author Devin Arena
 * @brief Handles the generation of tokens when reading the source string.
 * @since 5/20/2022
 **/

#ifndef PALLADIUM_SCANNER_H
#define PALLADIUM_SCANNER_H

#include "commons.h"
#include "value.h"

// Tokens found in the source string.
typedef enum {
  // Single-character tokens
  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_COMMA,
  TOKEN_DOT,
  TOKEN_MINUS,
  TOKEN_PLUS,
  TOKEN_SEMICOLON,
  TOKEN_SLASH,
  TOKEN_STAR,
  TOKEN_TILDE,
  TOKEN_TILDE_ARROW,
  // One or two character tokens
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  TOKEN_PLUS_EQUAL,
  TOKEN_MINUS_EQUAL,
  TOKEN_STAR_EQUAL,
  TOKEN_SLASH_EQUAL,
  TOKEN_AND,
  TOKEN_OR,
  TOKEN_DOUBLE_COLON,
  // Literals
  TOKEN_CHARACTER,
  TOKEN_IDENTIFIER,
  TOKEN_NUMBER_INTEGER,
  TOKEN_NUMBER_FLOATING,
  TOKEN_REFERENCE,
  TOKEN_STRING,
  // Keywords
  TOKEN_AS,
  TOKEN_BOOL,
  TOKEN_CHAR,
  TOKEN_DOUBLE,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_IMP,
  TOKEN_INST,
  TOKEN_INT,
  TOKEN_NSPACE,
  TOKEN_NULL,
  TOKEN_OBJ_CAST,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_STR,
  TOKEN_STRUCT,
  TOKEN_TRUE,
  TOKEN_VOID,
  TOKEN_WHILE,

  TOKEN_ERROR,
  TOKEN_EOF
} TokenType;

// Token contains information about the type, length, and what line a token is
// one.
typedef struct {
  TokenType type;
  const char* start;
  int length;
  int line;
} Token;

void initScanner(const char* source);
void insertSource(const char* source);
void appendSource(const char* source);
ValueType getValueTypeOfKeyword(TokenType type);
Token scanToken();

#endif