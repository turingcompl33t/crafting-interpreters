/**
 * @file scanner.h
 */

#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

/**
 * TokenType enumerates all token types
 * recognized by the Lox scanner.
 */
typedef enum {
  // Single character tokens
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

  // Single or double character tokens
  TOKEN_BANG, TOKEN_BANG_EQUAL,
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
  TOKEN_GREATER, TOKEN_GREATER_EQUAL,
  TOKEN_LESS, TOKEN_LESS_EQUAL,

  // Literals
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

  // Reserved keywords
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

  // Scanner control tokens
  TOKEN_ERROR, TOKEN_EOF
} TokenType;

/**
 * The Token class represents a token scanned
 * from a Lox source program.
 */
typedef struct {
  /** The type of the token */
  TokenType type;
  /** A pointer to the start of the token lexeme */
  const char* start;
  /** The length of the token lexeme */
  int length;
  /** The line number on which the token appears */
  int line;
} Token;

/**
 * Initialize the scanner.
 * @param source The Lox source program
 */
void initScanner(const char* source);

/**
 * Scan the next token from the source program.
 * @return The next token
 */
Token scanToken();

#endif // CLOX_SCANNER_H
