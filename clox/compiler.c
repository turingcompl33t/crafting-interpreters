/**
 * @file compiler.c
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

/**
 * The Parser type stores the state maintained during parsing.
 */
typedef struct {
  /** The current token */
  Token current;
  /** The previous token */
  Token previous;
  /** Denotes whether an error has occurred */
  bool hadError;
  /** Denotes whether the parser is in a bad state */
  bool panicMode;
} Parser;

/**
 * The Precedence enumeration defines the 
 * precedence for expression evaluation.
 * 
 * Precedence values are arranged from lowest
 * precedence to highest precedence.
 */
typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // - !
  PREC_CALL,        // . ()
  PREC_PRIMARY     
} Precedence;

/**
 * The signature for a parser callback.
 */
typedef void (*ParseFn)();

/**
 * A ParseRule tells us, given a token type:
 * 
 * - The function to compile a prefix expression starting
 *   with a token of that type
 * - The function to compile an infix expression whose left
 *   operand is followed by a token of that type
 * - The precedence of an infix expression that uses the
 *   the token as an operator
 * 
 * A ParseRule therefore represents a single row in the parser table.
 */
typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

// The global parser instance
Parser parser;
// The chunk that is being compiled
Chunk* compilingChunk;

/**
 * @return A pointer to the chunk to which bytecode is written.
 */
static Chunk* currentChunk() {
  return compilingChunk;
}

// ----------------------------------------------------------------------------
// Error Reporting
// ----------------------------------------------------------------------------

/**
 * Report an error at the specified token.
 * @param token The token at which the error occurred
 * @param message The error message
 */
static void errorAt(Token* token, const char* message) {
  if (parser.panicMode) return;

  // Enter panic mode
  parser.panicMode = true;
  fprintf(stderr, "[line %d] Error", token->line);
  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  // Flip the flag to denote that an error occurred
  parser.hadError = true;
}

/**
 * Report an error at the location of the consumed token.
 * @param message The error message
 */
static void error(const char* message) {
  errorAt(&parser.previous, message);
}

/**
 * Report an error at the current token.
 * @param message The error message
 */
static void errorAtCurrent(const char* message) {
  errorAt(&parser.current, message);
}

// ----------------------------------------------------------------------------
// Internal Parser State Management
// ----------------------------------------------------------------------------

/**
 * Advance the parser state to the next non-error token.
 */
static void advance() {
  // Cache the current token
  parser.previous = parser.current;
  for (;;) {
    // Grab the next token from the scanner
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

/**
 * Consume a token of the specified type if it is the next
 * token in the stream, otherwise report an error.
 * @param type The token type
 * @param message The error message to report, if applicable
 */
static void consume(TokenType type, const char* message) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(message);
}

// ----------------------------------------------------------------------------
// Bytecode Generation
// ----------------------------------------------------------------------------

/**
 * Add a constant value to the chunk's constant pool.
 * @param value The constant value to add
 * @return The index in the constant pool at which
 * the new constant value was added
 */
static uint8_t makeConstant(Value value) {
  int constant = addConstant(currentChunk(), value);
  if (constant > UINT8_MAX) {
    error("Maximum number of constant values in chunk exceeded.");
    return 0;
  }
  return (uint8_t)constant;
}

/**
 * Emit a byte into the bytecode stream.
 * @param byte The byte
 */
static void emitByte(uint8_t byte) {
  writeChunk(currentChunk(), byte, parser.previous.line);
}

/**
 * Emit a pair of bytes into the bytecode stream.
 * @param b0 The first byte to emit
 * @param b1 The second byte to emit
 */
static void emitBytes(uint8_t b0, uint8_t b1) {
  emitByte(b0);
  emitByte(b1);
}

/**
 * Emit a return instruction into the bytecode stream.
 */
static void emitReturn() {
  emitByte(OP_RETURN);
}

/**
 * Emit a constant into the bytecode stream.
 * @param value The constant value
 */
static void emitConstant(Value value) {
  // Emit both the opcode and the index from
  // which the constant is loaded
  emitBytes(OP_CONSTANT, makeConstant(value));
}

/**
 * Terminate the compilation process.
 */
static void endCompiler() {
  emitReturn();

#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError) {
    disassembleChunk(currentChunk(), "code");
  }
#endif
}

// ----------------------------------------------------------------------------
// Pratt Parser
// ----------------------------------------------------------------------------

/* Forward Declarations */
static void expression();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

/**
 * Emit the bytecode to load a number literal into the bytecode stream.
 */
static void number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

/**
 * Emit the bytecode to load a string literal into the bytecode stream.
 */
static void string() {
  emitConstant(
    OBJECT_VAL(
      copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Emit the bytecode to evaluate a unary 
 * expression into the bytecode stream.
 */
static void unary() {
  // The leading token has been consumed
  TokenType operatorType = parser.previous.type;

  // Compile the operand
  parsePrecedence(PREC_UNARY);

  // NOTE: We emit the instruction for the operator
  // AFTER emitting the instruction(s) for the operand
  // because of the stack-based nature of our VM

  // Emit the operator instruction
  switch (operatorType) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default: return; // Unreachable
  }
}

/**
 * Emit the bytecode to evaluate a binary
 * expression into the bytecode stream.
 */
static void binary() {
  TokenType operatorType = parser.previous.type;
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // NOTE: Below we implement some of the logical binary
  // operators in terms of others that are already defined;
  // for instance a <= b <=> !(a > b)

  switch (operatorType) {
    case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT);
    case TOKEN_LESS:          emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT);
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default: return; // Unreachable
  }
}

/**
 * Emit the bytecode to evaluate a grouping 
 * expression into the bytecode stream.
 */
static void grouping() {
  // NOTE: A grouping expression is purely a syntactic
  // construct - it has no runtime semantics, so as far
  // as the backend (bytecode generation) is concerned,
  // there is nothing to do here other than recursively
  // call into expression() to generate code for the
  // expression that is contained within the parentheses
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

/**
 * Emit the bytecode to evaluate a literal expression
 * into the bytecode stream.
 */
static void literal() {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_TRUE:  emitByte(OP_TRUE); break;
    case TOKEN_NIL:   emitByte(OP_NIL); break;
    default: return; // Unreachable
  }
}

/**
 * Emit the bytecode for an expression into the bytecode stream.
 */
static void expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

/**
 * The parser table.
 */
ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_LESS]          = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
  [TOKEN_STRING]        = {string,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE}
};

/**
 * Emit bytecode for the current expression, terminating
 * when the precedence for the expression falls below
 * the threshold specified by `precedence`.
 * 
 * The parsePrecedence() function implements the core
 * logic of the Pratt parser.
 * 
 * @param precedence The threshold precedence
 */
static void parsePrecedence(Precedence precedence) {
  // High-level description of the Pratt parser algorithm:
  //
  // The first token always belongs to some kind of prefix
  // expression, by definition, so we first consult the parser
  // table to lookup the prefix rule for this expression.
  //
  // Once the prefix rule is exhausted, we then look for an
  // infix rule for the next token. If we find one, the prefix
  // expression we already compiled might be an operand for
  // it, but ONLY if the call to parsePrecedence() has a 
  // precedence that is low enough to permit the infix operator.
  
  advance();

  // If no prefix rule is present for the token,
  // this token must represent a syntax error
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Expected expression.");
    return;
  }

  prefixRule();

  // The parsing algorithm continues so long as the specified
  // precedence is less or equal to the precendece of the parse
  // rule specified for the current token; this allows us to
  // produce the correct parse result, observing operator precedence.
  //
  // A great example for walking through this logic is considering
  // how the parse of -1 + 2 proceeds. In this case, we need to
  // ensure that the '-' is bound to `1` alone and NOT the result
  // of evaluating the expression `1 + 2`. The fact that the unary()
  // parse function calls back into parsePrecedence() with PREC_UNARY
  // as the precedence ensures that the binary expression is not
  // evaluated - the numeric literal `1` is evaluated (emitting an
  // OP_CONSTANT bytecode into the stream) but then when we reach
  // the check immediately below this comment, we do not consider
  // any of the infix rules because the precedence PREC_UNARY is 
  // not less than the precedence of the next token `+` PREC_TERM.
  
  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    infixRule();
  }
}

/**
 * Get the parse rule for the specified token type.
 * @param type The token type
 * @return The corresponding parse rule
 */
static ParseRule* getRule(TokenType type) {
  return &rules[type];
}

// ----------------------------------------------------------------------------
// Compilation Interface
// ----------------------------------------------------------------------------

bool compile(const char* source, Chunk* chunk) {
  initScanner(source);
  compilingChunk = chunk;

  parser.hadError = false;
  parser.panicMode = false;

  advance();
  expression();

  consume(TOKEN_EOF, "Expect end of expression.");
  endCompiler();

  return !parser.hadError;
}
