/**
 * @file scanner.c
 * @author Devin Arena
 * @brief Scans tokens from the input file for the compiler implementation.
 * @since 5/20/2022
 **/

#include <stdio.h>
#include <string.h>

#include "scanner.h"

// handles scanner logic
typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner scanner;

/**
 * @brief Initializes the scanner object with the specified source string.
 *
 * @param source const char* the source string to scan.
 */
void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

/**
 * @brief Helper method for determining if a character is a digit.
 *
 * @param c char the character to analyze
 * @return bool true if the character is a digit, false otherwise.
 */
static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * @brief Helper method for determining if a character is a letter.
 *
 * @param c char the character to analyze
 * @return bool true if the character is a letter, false otherwise.
 */
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @brief Helper for determining if the current character is the EOF character.
 *
 * @return bool true if current character is EOF, false otherwise.
 */
static bool isAtEnd() {
  return *scanner.current == '\0';
}

/**
 * @brief Advances the scanner to the next character, returning the current.
 *
 * @return char the current character before advancing.
 */
static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

/**
 * @brief Returns the current character.
 *
 * @return char the current character.
 */
static char peek() {
  return *scanner.current;
}

/**
 * @brief Returns the next character.
 *
 * @return char the next character
 */
static char peekNext() {
  // if at end, return EOF
  if (isAtEnd())
    return '\0';
  return scanner.current[1];
}

/**
 * @brief Checks if the current character is the specified character. If so, the
 * scanner advances.
 *
 * @param expected char the expected character
 * @return bool true if the current character is the expected character, false
 * otherwise.
 */
static bool match(char expected) {
  if (isAtEnd())
    return false;
  if (*scanner.current != expected)
    return false;
  scanner.current++;
  return true;
}

/**
 * @brief Helper method for generating a token with the specified type.
 *
 * @param type TokenType the type of the token
 * @return Token the token with the specified type
 */
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int)(scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

/**
 * @brief Helper method for generating an error token.
 *
 * @param message the error message to display
 * @return Token the error token
 */
static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

/**
 * @brief Helper method for skipping whitespace (also handles comments.)
 */
static void skipWhitespace() {
  while (true) {
    char c = peek();
    switch (c) {
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;
      case '\n':
        scanner.line++;
        advance();
        break;
      case '/':
        if (peekNext() == '/') {
          while (peek() != '\n' && !isAtEnd())
            advance();
        } else {
          return;
        }
        break;
      default:
        return;
    }
  }
}

/**
 * @brief Checks if the remainder of a string is a keyword. If so, returns the
 * keyword token. Otherwise, this token is an identifier.
 *
 * @param start the start position in the string
 * @param length the length of the string
 * @param rest the rest to compare to
 * @param type the type of the token if it is a keyword
 * @return TokenType the type of the token if it is a keyword or
 * TOKEN_IDENTIFIER if it is an identifier
 */
static TokenType checkKeyword(int start,
                              int length,
                              const char* rest,
                              TokenType type) {
  // check the substring with the specified string with memcmp
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

/**
 * @brief Generates a token for the identifier. If its a keyword, this returns
 * the keyword token type, otherwise, it returns an identifier token.
 *
 * @return TokenType the type of the token if it is a keyword or
 * TOKEN_IDENTIFIER if it is an identifier
 */
static TokenType identifierType() {
  switch (scanner.start[0]) {
    case 'b':
      return checkKeyword(1, 3, "ool", TOKEN_BOOL);
    case 'c':
      return checkKeyword(1, 3, "har", TOKEN_CHAR);
    case 'd':
      return checkKeyword(1, 5, "ouble", TOKEN_DOUBLE);
    case 'e':
      return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a':
            return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o':
            return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u':
            return checkKeyword(2, 1, "n", TOKEN_VOID);
          default:
            return TOKEN_IDENTIFIER;
        }
      }
      break;
    case 'i':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'f':
            return checkKeyword(1, 1, "f", TOKEN_IF);
          case 'n':
            return checkKeyword(2, 1, "t", TOKEN_INT);
          default:
            return TOKEN_IDENTIFIER;
        }
      }
      break;
    case 'n':
      return checkKeyword(1, 3, "ull", TOKEN_NULL);
    case 'p':
      return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
      return checkKeyword(1, 2, "et", TOKEN_RETURN);
    case 's':
      if (scanner.current - scanner.start == 3) {
        return checkKeyword(1, 2, "tr", TOKEN_STR);
      }
      return checkKeyword(1, 5, "truct", TOKEN_STRUCT);
    case 't':
      return checkKeyword(2, 2, "ue", TOKEN_TRUE);
    case 'v':
      return checkKeyword(1, 3, "oid", TOKEN_VOID);
    case 'w':
      return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    default:
      return TOKEN_IDENTIFIER;
  }
}

/**
 * @brief Helper for generating an identifier or keyword from the scanner.
 *
 * @return Token an identifier token or keyword token
 */
static Token identifier() {
  // identifiers can be letters or numbers
  while (isAlpha(peek()) || isDigit(peek()))
    advance();

  return makeToken(identifierType());
}

/**
 * @brief Helper for generating a number from the scanner.
 *
 * @return Token a number token
 */
static Token number() {
  // handle ints
  while (isDigit(peek()))
    advance();

  // for handling decimals
  if (peek() == '.' && isDigit(peekNext())) {
    advance();

    while (isDigit(peek()))
      advance();

    return makeToken(TOKEN_NUMBER_FLOATING);
  }

  return makeToken(TOKEN_NUMBER_INTEGER);
}

/**
 * @brief Generates a string token from the scanner.
 *
 * @return Token a string token
 */
static Token string() {
  // multi-line string support
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }

  // strings must be terminated.
  if (isAtEnd())
    return errorToken("Unterminated string.");

  advance();  // The closing ".
  return makeToken(TOKEN_STRING);
}

/**
 * @brief Recursively generates a token from the scanner based on the source
 * string.
 *
 * @return Token the proper token for whats in the source string, or an error if
 * its not a valid token
 */
Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();

  if (isAlpha(c))
    return identifier();
  if (isDigit(c))
    return number();

  switch (c) {
    case '(':
      return makeToken(TOKEN_LEFT_PAREN);
    case ')':
      return makeToken(TOKEN_RIGHT_PAREN);
    case '{':
      return makeToken(TOKEN_LEFT_BRACE);
    case '}':
      return makeToken(TOKEN_RIGHT_BRACE);
    case ';':
      return makeToken(TOKEN_SEMICOLON);
    case ',':
      return makeToken(TOKEN_COMMA);
    case '.':
      return makeToken(TOKEN_DOT);
    case '-':
      return makeToken(TOKEN_MINUS);
    case '+':
      return makeToken(TOKEN_PLUS);
    case '/':
      return makeToken(TOKEN_SLASH);
    case '*':
      return makeToken(TOKEN_STAR);
    case '&':
      if (match('&'))
        return makeToken(TOKEN_AND);
      return makeToken(TOKEN_REFERENCE);
    case '|':
      if (match('|'))
        return makeToken(TOKEN_OR);
      break;
    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '\'': {
      advance();
      Token tchar = makeToken(TOKEN_CHARACTER);
      if (!match('\''))
        return errorToken("Expected ' after character.");
      return tchar;
    }

    case '"':
      return string();
  }

  return errorToken("Unexpected character.");
}

/**
 * @brief Lookup table for keywords to get their required valueType.
 */
ValueType keywordTypes[] = {[TOKEN_INT] = VALUE_INTEGER,
                            [TOKEN_DOUBLE] = VALUE_DOUBLE,
                            [TOKEN_BOOL] = VALUE_BOOL,
                            [TOKEN_CHARACTER] = VALUE_CHARACTER,
                            [TOKEN_IDENTIFIER] = VALUE_OBJECT};

/**
 * @brief Helper to lookup the valueType of a keyword.
 *
 * @param type TokenType of the keyword
 * @return ValueType the valueType of the keyword
 */
ValueType getValueTypeOfKeyword(TokenType type) {
  return keywordTypes[type];
}