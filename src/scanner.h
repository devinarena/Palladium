/**
 * @file scanner.h
 * @author Devin Arena
 * @brief Handles the generation of tokens when reading the source string.
 * @since 5/20/2022
 **/

#ifndef PALLADIUM_SCANNER_H
#define PALLADIUM_SCANNER_H

#include "commons.h"

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
  // One or two character tokens
  TOKEN_BANG,
  TOKEN_BANG_EQUAL,
  TOKEN_EQUAL,
  TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER,
  TOKEN_GREATER_EQUAL,
  TOKEN_LESS,
  TOKEN_LESS_EQUAL,
  // Literals
  TOKEN_IDENTIFIER,
  TOKEN_STRING,
  TOKEN_NUMBER_INTEGER,
  TOKEN_NUMBER_FLOATING,
  TOKEN_CHARACTER,
  TOKEN_REFERENCE,
  // Keywords
  TOKEN_AND,
  TOKEN_BOOL,
  TOKEN_DOUBLE,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FOR,
  TOKEN_FUN,
  TOKEN_IF,
  TOKEN_INT,
  TOKEN_NULL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
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
Token scanToken();

#endif