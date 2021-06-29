/**
 * @file compiler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
typedef void (*ParseFn)(bool canAssign);

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

/**
 * The Local type represents a local variable.
 */
typedef struct {
  /** The local identifier */
  Token name;
  /** The depth of the scope in which the local appears */
  int depth;
} Local;

/**
 * The Compiler type maintains state during the compilation process.
 */
typedef struct {
  /** A flat array of in-scope locals */
  Local locals[UINT8_COUNT];
  /** The number of locals in scope */
  int localCount;
  /** The current scope depth */
  int scopeDepth;
} Compiler;

/** The global parser instance */
Parser parser;
/** The global compiler instance */
Compiler* current = NULL;
/** The chunk that is being compiled */
Chunk* compilingChunk;

/**
 * @return A pointer to the chunk to which bytecode is written.
 */
static Chunk* currentChunk() {
  return compilingChunk;
}

// ----------------------------------------------------------------------------
// Compiler Initialization
// ----------------------------------------------------------------------------

/** Forward declarations */
static void emitByte(uint8_t byte);

/**
 * Initialize the global compiler instance with `compiler`.
 */
static void initCompiler(Compiler* compiler) {
  compiler->localCount = 0;
  compiler->scopeDepth = 0;
  current = compiler;
}

/**
 * Enter a new lexical scope.
 */
static void beginScope() {
  current->scopeDepth++;
}

/**
 * Exit the current lexical scope.
 */
static void endScope() {
  current->scopeDepth--;

  // While we have not reached the end of the locals array
  // AND we have not found a local in a scope that is at the
  // same depth as the NEW scope depth, continue to pop locals
  while (current->localCount > 0 &&
    current->locals[current->localCount - 1].depth > current->scopeDepth) {
    emitByte(OP_POP);
    current->localCount--;
  }
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

/**
 * Determine if the current token matches the specified type.
 * @param type The token type of interest
 * @return `true` if the current token matches, `false` otherwise
 */
static bool check(TokenType type) {
  return parser.current.type == type;
}

/**
 * Determine if the current token matches the specified
 * type; if so, consume the current token.
 * @param type The token type of interest
 * @return `true` if the current token matches, `false` otherwise
 */
static bool match(TokenType type) {
  if (!check(type)) return false;
  advance();
  return true;
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
 * Make an identifier (string) constant for the specified token.
 * @param name The token the specifies the identifier name
 * @return The index in the constant pool at which
 * the allocated identifier constant is located
 */
static uint8_t identifierConstant(Token* name) {
  return makeConstant(OBJECT_VAL(copyString(name->start, name->length)));
}

/**
 * Determine the equality of two token names.
 * @param a Input token
 * @param b Input token
 * @return `true` if tokens `a` and `b` have the
 * same identifier, `false` otherwise
 */
static bool identifierEquals(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

/**
 * Resolve a local variable.
 * @param compiler The compiler state
 * @param name The token that identifies the local to resolve
 * @return The index of the local in the locals array,
 * or -1 in the event the local cannot be resolved
 */
static int resolveLocal(Compiler* compiler, Token* name) {
  for (int i = compiler->localCount - 1; i >= 0; i--) {
    Local* local = &compiler->locals[i];
    if (identifierEquals(name, &local->name)) {
      if (local->depth == -1) {
        error("Can't read local variable in its own initializer.");
      }
      return i;
    }
  }
  return -1;
}

/**
 * Add a local variable to the locals array.
 * @param name The name of the local variable
 */
static void addLocal(Token name) {
  if (current->localCount == UINT8_COUNT) {
    error("Too many local variables in scope.");
    return;
  }

  Local* local = &current->locals[current->localCount++];
  local->name = name;
  // Set the local's depth to a sentinel value before initialization
  local->depth = -1;
}

/**
 * Mark the most-recently defined local variable as initialized.
 * 
 * This function completes the 2-step process of delcaration and definition.
 */
static void markInitialized() {
  current->locals[current->localCount - 1].depth = current->scopeDepth;
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
 * Emit a jump instruction into the bytecode stream.
 * @param opcode The opcode for the jump to emit
 * @return The address of the emitted jump instruction
 */
static int emitJump(uint8_t opcode) {
  emitByte(opcode);

  // NOTE: Here we limit ourselves to 2 bytes with
  // which we can specify the jump offset; this limits
  // the range of the jump targets we can take, and is
  // why some architectures have distinct jump and
  // "long jump" instructions
  emitByte(0xFF);
  emitByte(0xFF);

  // Return the address of the start of the jump
  return currentChunk()->count - 2;
}

/**
 * Patch the jump instruction at `offset` with a jump target
 * that jumps to the current location in the bytecode stream.
 * @param offset The address of the jump instruction to patch
 */
static void patchJump(int offset) {
  // Compute the jump target;
  // -2 to account for jump target itself
  int jump = currentChunk()->count - offset - 2;

  if (jump > UINT16_MAX) {
    error("Jump offset too large.");
  }

  // Perform the patch
  currentChunk()->code[offset] = (jump >> 8) & 0xFF;
  currentChunk()->code[offset + 1] = jump & 0xFF;
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
// Statements
// ----------------------------------------------------------------------------

/**
 * Synchronize the parser and compiler state after a panic.
 */
static void synchronize() {
  parser.panicMode = false;
  // Consume tokens until we hit something
  // that looks like a statement boundary
  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) return;
    switch (parser.current.type) {
      case TOKEN_CLASS:
      case TOKEN_FUN:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
        return;
      default:
        ; // Do nothing
    }

    advance();
  }
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
 * @param canAssign Ignored
 */
static void number(bool canAssign) {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value));
}

/**
 * Emit the bytecode to load a string literal into the bytecode stream.
 * @param canAssign Ignored
 */
static void string(bool canAssign) {
  emitConstant(
    OBJECT_VAL(
      copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

/**
 * Emit the bytecode to load a named variable into the bytecode stream.
 * @param name The name of the variable
 * @param canAssign `true` if the variable being parsed can
 * be assigned to, `false` otherwise
 */
static void namedVariable(Token name, bool canAssign) {
  uint8_t getOp;
  uint8_t setOp;
  int arg = resolveLocal(current, &name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

/**
 * Emit the bytecode to load variable into the bytecode stream.
 * @param canAssign `true` if the variable being parsed can
 * be assigned to, `false` otherwise
 */
static void variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

/**
 * Emit the bytecode to evaluate a unary 
 * expression into the bytecode stream.
 * @param canAssign Ignored
 */
static void unary(bool canAssign) {
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
 * @param canAssign Ignored
 */
static void binary(bool canAssign) {
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
 * @param canAssign Ignored
 */
static void grouping(bool canAssign) {
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
 * @param canAssign Ignored
 */
static void literal(bool canAssign) {
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
  [TOKEN_IDENTIFIER]    = {variable, NULL,   PREC_NONE},
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

  bool canAssign = (precedence <= PREC_ASSIGNMENT);
  prefixRule(canAssign);

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
    infixRule(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
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
// Statements
// ----------------------------------------------------------------------------

/** Forward declarations for statement productions */
static void statement();
static void declaration();

/**
 * Emit the bytecode to define a global variable into the bytecode stream.
 * @param global The index of the global in the constant pool
 */
static void defineVariable(uint8_t global) {
  // Elide the code to define a local at runtime
  if (current->scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, global);
}

/**
 * Declare a local variable (locals only).
 */
static void declareVariable() {
  if (current->scopeDepth == 0) return;
  Token* name = &parser.previous;
  // Start at the end of the locals array and work back
  for (int i = current->localCount - 1; i >= 0; --i) {
    Local* local = &current->locals[i];
    // Break the loop when we find a local that has a depth
    // that is less than the current scope depth
    if (local->depth != -1 && local->depth < current->scopeDepth) {
      break;
    }
    if (identifierEquals(name, &local->name)) {
      error("Already a variable with this name in scope.");
    }
  }

  addLocal(*name);
}

/**
 * Parse a variable name and create a constant
 * value entry for the named variable.
 * @param errorMessage The error message to report
 * @return The index in the constant pool at which
 * the allocated identifier constant is located
 */
static uint8_t parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);

  declareVariable();
  // Exit the function if in a local scope
  if (current->scopeDepth > 0) return 0;

  // Only globals are looked up at runtime by name,
  // so only globals have their names added to the
  // constant pool for runtime lookup
  return identifierConstant(&parser.previous);
}

/**
 * Emit the bytecode for a print statement into the bytecode stream.
 */
static void printStatement() {
  // The TOKEN_PRINT token is already consumed
  // prior to the call into this function
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after value.");
  emitByte(OP_PRINT);
}

/**
 * Emit the bytecode for a conditional branch into the bytecode stream.
 */
static void ifStatement() {
  // Compile the expression for the condition; the result
  // of evaluting this expression is left at the stack top
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after 'if'.");

  // Emit the jump bytecode first with a dummy jump offset;
  // track the location of the jump so we can patch later
  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  // Emit the instruction to pop the condition result
  // from the stack at the start of the 'then' branch
  emitByte(OP_POP);
  statement();

  // Emit a jump to skip over the 'else' branch at the 
  // end of the 'then' branch
  int elseJump = emitJump(OP_JUMP);

  // Backpatch the correct jump target
  patchJump(thenJump);

  // Emit the instruction to pop the condition result
  // from, the stack at the start of the 'else' branch
  emitByte(OP_POP);

  // If an 'else' branch is present, compile it
  if (match(TOKEN_ELSE)) statement();

  // Finally, path the unconditional 'else' jump
  patchJump(elseJump);
}

/**
 * Emit the bytecode for a block statement into the bytecode stream.
 */
static void block() {
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }
  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

/**
 * Emit the bytecode for an expression statement into the bytecode stream.
 */
static void expressionStatement() {
  // Semantically, an expression statement evaluates the
  // expression and discards the result
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

/**
 * Emit the bytecode for a variable declaration into the bytecode stream.
 */
static void varDeclaration() {
  // Variable declaration proceeds in 3 high-level steps:
  //  1. Parse the variable identifier and add it to the constant pool
  //  2. Parse the initializer; emit the bytecode to push the result of 
  //     the initializer expression onto the top of the stack
  //  3. Emit the bytecode to define the variable itself
  //
  // After compilation, at runtime, this means that we can get the
  // name of the variable from the constant pool (index is encoded)
  // directly into the instruction) and then look at the top of the
  // stack to locate the initializer value for the variable

  uint8_t global = parseVariable("Expect variable name.");
  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }

  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
  defineVariable(global);
}

/**
 * Emit the bytecode for a statement into the bytecode stream.
 */
static void statement() {
  if (match(TOKEN_PRINT)) {
    printStatement();
  } else if (match(TOKEN_IF)) {
    ifStatement();
  } else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();
    block();
    endScope();
  } else {
    expressionStatement();
  }
}

/**
 * Emit the bytecode for a declaration into the bytecode stream. 
 */
static void declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }

  // If we hit a compilation error while compiling
  // a declaration, synchronize the compiler
  if (parser.panicMode) synchronize();
}

// ----------------------------------------------------------------------------
// Compilation Interface
// ----------------------------------------------------------------------------

bool compile(const char* source, Chunk* chunk) {
  initScanner(source);

  Compiler compiler;
  initCompiler(&compiler);

  compilingChunk = chunk;

  parser.hadError = false;
  parser.panicMode = false;

  advance();

  while (!match(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_EOF, "Expect end of expression.");
  endCompiler();

  return !parser.hadError;
}
