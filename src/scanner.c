/**
 * @file scanner.c
 * @author Devin Arena
 * @brief Scans tokens from the input file for the compiler implementation.
 * @since 5/20/2022
 **/

#include "scanner.h"

typedef struct {
  const char* start;
  const char* current;
  int line;
} Scanner;

Scanner scanner;

/**
 * @brief Initializes the scanner with the source string.
 *
 * @param source const char* The source string to scan.
 */
void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

/**
 * @brief Helper for determining if a character is a digit.
 *
 * @param c char The character to check.
 * @return bool True if the character is a digit, false otherwise.
 */
static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * @brief Helper for determining if a character is a letter.
 *
 * @param c char The character to check.
 * @return bool True if the character is a letter, false otherwise.
 */
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

/**
 * @brief Helper for determining if we are at the end of the source string.
 *
 * @return bool True if the current character is a null terminator, false
 * otherwise.
 */
static bool isAtEnd() {
  return *scanner.current == '\0';
}

/**
 * @brief Advance the scanner to the next character, returning the current one.
 *
 * @return char The current character BEFORE advancing.
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
 * @brief Returns the next character, or null terminator if at the end of the
 * source.
 *
 * @return char the next character.
 */
static char peekNext() {
  if (isAtEnd())
    return '\0';
  return scanner.current[1];
}

/**
 * @brief Advances the scanner if the current character is equal to the given
 * one. Returns true if the current character is equal to the given one, false
 * otherwise.
 *
 * @param expected char The character to compare to.
 * @return bool True if the current character is equal to the given one, false
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
 * @brief Creates a token from the given type at the current scanner position.
 *
 * @param type TokenType The type of token to create.
 * @return Token The created token.
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
 * @brief Creates an error token with the given message.
 *
 * @param message const char* The message to include in the error.
 * @return Token The error token.
 */
static Token errorToken(const char* message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = scanner.start;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

/**
 * @brief Handles number token creatopm.
 *
 * @return Token The created number token.
 */
static Token number() {
  while (isDigit(peek()))
    advance();

  if (peek() == '.' && isDigit(peekNext())) {
    advance();

    while (isDigit(peek()))
      advance();
  }

  return makeToken(TOKEN_NUMBER);
}

/**
 * @brief Handles string token creation.
 *
 * @return Token The created string token.
 */
static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      scanner.line++;
    advance();
  }

  if (isAtEnd())
    return errorToken("Unterminated string.");

  // The closing ".
  advance();
  return makeToken(TOKEN_STRING);
}

/**
 * @brief Skips whitespace characters and comments.
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
 * @brief Determines if the token is a keyword or identifier.
 *
 * @param start const char* The start of the token.
 * @param length int The length of the token.
 * @param rest const char* The rest of the token.
 * @param type TokenType The type of the token.
 * @return TokenType The type of the token.
 */
static TokenType checkKeyword(int start,
                              int length,
                              const char* rest,
                              TokenType type) {
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }

  return TOKEN_IDENTIFIER;
}

/**
 * @brief Returns the keyword type for the given token or an identifier.
 *
 * @return TokenType The type of the token.
 */
static TokenType identifierType() {
  switch (scanner.start[0]) {
    case 'a':
      return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c':
      return checkKeyword(1, 4, "lass", TOKEN_CLASS);
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
            return checkKeyword(2, 1, "n", TOKEN_FUN);
          default:
            return TOKEN_IDENTIFIER;
        }
      }
      break;
    case 'i':
      switch (scanner.start[1]) {
        case 'f':
          return TOKEN_IF;
        case 'n':
          return checkKeyword(2, 1, "t", TOKEN_INT);
        default:
          return TOKEN_IDENTIFIER;
      }
      break;
    case 'n':
      return checkKeyword(1, 3, "ull", TOKEN_NULL);
    case 'o':
      return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p':
      return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r':
      return checkKeyword(1, 2, "et", TOKEN_RETURN);
    case 's':
      return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h':
            return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r':
            return checkKeyword(2, 2, "ue", TOKEN_TRUE);
          default:
            return TOKEN_IDENTIFIER;
        }
      }
      break;
    case 'v':
      return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w':
      return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    default:
      return TOKEN_IDENTIFIER;
  }
}

/**
 * @brief Handles identifier token creation.
 *
 * @return Token The created identifier token.
 */
static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek()))
    advance();

  return makeToken(identifierType());
}

/**
 * @brief Scans the next token in the file, producing EOF if none are left.
 *
 * @return Token The next token in the file.
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

    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

    case '"':
      return string();
  }

  return errorToken("Unexpected character.");
}