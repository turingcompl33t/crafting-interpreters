/**
 * @file scanner.c
 */

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
  /** The start of the current lexeme being scanned */
  const char* start;
  /** The current position in the current lexeme */
  const char* current;
  /** The current line number */
  int line;
} Scanner;

// The global scanner for the module
Scanner scanner;

// ----------------------------------------------------------------------------
// Scanner Lifetime
// ----------------------------------------------------------------------------

void initScanner(const char* source) {
  scanner.start = source;
  scanner.current = source;
  scanner.line = 1;
}

// ----------------------------------------------------------------------------
// scanToken() Low-Level Utilities
// ----------------------------------------------------------------------------

/**
 * Determine if we have reached the end of the input program.
 * @return `true` if scanner has reached end of input, `false` otherwise
 */
static bool isAtEnd() {
  return *scanner.current == '\0';
}

/**
 * Consume a character and return it.
 * @return The next character from the input program source
 */
static char advance() {
  scanner.current++;
  return *(scanner.current - 1);
}

/**
 * Determine if the current character matches `expected`
 * and consume the current token it if so.
 * @param expected The expected character
 * @return `true` if the current character matches `expected`, `false` otherwise
 */
static bool match(char expected) {
  if (isAtEnd()) return false;
  if (*scanner.current != expected) return false;
  // Consume the token if a match
  scanner.current++;
  return true;
}

/**
 * Peek at the scanner's cursor without consuming it.
 * @return The character at the scanner's cursor
 */
static char peek() {
  return *scanner.current;
}

/**
 * Peek at the character just beyond the scanner's cursor.
 * @return The character at cursor + 1
 */
static char peekNext() {
  if (isAtEnd()) return '\0';
  return *(scanner.current + 1);
}

/**
 * Determine if the character is a decimal digit.
 * @param c The character
 * @return `true` if `c` is a decimal digit, `false` otherwise
 */
static bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

/**
 * Determine if the character is alphabetical.
 * @param c The character
 * @return `true` if `c` is ASCII alphabetical, `false` otherwise
 */
static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || 
         (c >= 'A' && c <= 'z') || 
         c == '_';
}

/**
 * Advance the scanner cursor over whitespace until
 * the next non-whitespace character is found.
 */
static void skipWhitespace() {
  for (;;) {
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
          while (peek() != '\n' && !isAtEnd()) advance();
        } else {
          return;
        }
      default:
        return;
    }
  }
}

/**
 * Make a token of the given type, based on the current
 * state of the global scanner instance.
 * @param type The token type
 * @return The new token
 */
static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;
  token.length = (int) (scanner.current - scanner.start);
  token.line = scanner.line;
  return token;
}

/**
 * Make an error token with the given error message.
 * @param message The error message
 * @return The new token
 */
static Token errorToken(const char* message) {
  // NOTE: It feels a little dirty to just
  // assume that the lifetime of the string
  // is managed elsewhere, but this is really
  // only ever called with string literals
  
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int)strlen(message);
  token.line = scanner.line;
  return token;
}

// ----------------------------------------------------------------------------
// scanToken() High-Level Utilities
// ----------------------------------------------------------------------------

/**
 * Determine if the remainder of the current lexeme
 * matches the string specified by `rest`.
 * @param start The start index in the current lexeme
 * @param length The length of `rest`
 * @param rest The residual string to check
 * @param type The type of the token, if a match is made
 * @return The type of the token
 */
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
  // We need to ensure that the current lexeme is
  // exactly as long as the keyword against which
  // we are comparing; we are able to check this
  // because by this point the entire lexeme has
  // been consumed by the scanner, so the cursor
  // has advanced past the end of the lexeme
  if (scanner.current - scanner.start == start + length &&
    memcmp(scanner.start + start, rest, length) == 0) {
    return type;
  }
  return TOKEN_IDENTIFIER;
}

/**
 * Determine the token type for an identifier token,
 * checking reserved words.
 * @return The type type
 */
static TokenType identifierType() {
  // NOTE: Here, we implement a (very) ad-hoc trie
  // data structure that we probe to determine if
  // an identifier is a reserved keyword
  switch (scanner.start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "nil", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }
  return TOKEN_IDENTIFIER;
}

/**
 * Scan an identifier from the program source.
 * @return The identifier token
 */
static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek())) advance();
  return makeToken(identifierType());
}

/**
 * Scan a string literal from the program source.
 * @return The string token
 */
static Token string() {
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') scanner.line++;
    advance();
  }
  
  if (isAtEnd()) return errorToken("Unterminated string.");
  
  // Consume the closing '"'
  advance();
  return makeToken(TOKEN_STRING);
}

/**
 * Scan a numeric literal from the program source.
 * @return The number token
 */
static Token number() {
  // Consume all leading digits
  while (isDigit(peek())) advance();

  // Check for the decimal point
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the decimal point
    advance();
    // Consume all digits beyond the decimal
    while (isDigit(peek())) advance();
  }
  return makeToken(TOKEN_NUMBER);
}

// ----------------------------------------------------------------------------
// Token Scanning
// ----------------------------------------------------------------------------

/**
 * Scan the next token from the program source.
 * @return The token
 */
Token scanToken() {
  skipWhitespace();
  scanner.start = scanner.current;
  if (isAtEnd()) return makeToken(TOKEN_EOF);

  char c = advance();
  if (isAlpha(c)) return identifier();
  if (isDigit(c)) return number();

  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
    case '!':
      return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string();
  }

  return errorToken("Unexpected character");
}
