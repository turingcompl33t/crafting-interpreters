/**
 * Parser.java
 */

package com.craftinginterpreters.lox;

import java.util.List;
import java.util.ArrayList;

import com.craftinginterpreters.lox.ast.*;

public class Parser {
  /**
   * The ParseError class denotes an error during parsing.
   */
  private static class ParseError extends RuntimeException {}

  /**
   * The collection of scanned tokens.
   */
  private final List<Token> tokens;

  /**
   * A pointer to the current token.
   */
  private int current = 0;

  /**
   * Construct a new parser instance.
   * @param tokens The collection of tokens to parse.
   */
  Parser(final List<Token> tokens) {
    this.tokens = tokens;
  }

  /**
   * Parse the input token sequence to a Lox AST.
   * @return The list of statements in the AST
   */
  public List<Stmt> parse() {
    List<Stmt> statements = new ArrayList<>();
    while (!isAtEnd()) {
      statements.add(declaration());
    }
    return statements;
  }

  // --------------------------------------------------------------------------
  // Statement Productions
  // --------------------------------------------------------------------------

  /**
   * Build the syntax tree for the `declaration` production.
   * @return The `declaration` syntax tree
   */
  private Stmt declaration() {
    try {
      if (match(TokenType.VAR)) return varDeclaration();
      return statement();
    } catch (ParseError error) {
      synchronize();
      return null;
    }
  }

  /**
   * Build the syntax tree for the `var declaration` production.
   * @return The `var declaration` syntax tree
   */
  private Stmt varDeclaration() {
    final Token name = consume(TokenType.IDENTIFER, "Expect a variable name.");
    // Parse an initializer if we see an '='
    final Expr initializer = match(TokenType.EQUAL) ? expression() : null;
    consume(TokenType.SEMICOLON, "Expect ';' after variable declaration.");
    return new VarStmt(name, initializer);
  }

  /**
   * Build the syntax tree for the `statement` production.
   * @return The `statement` syntax tree
   */
  private Stmt statement() {
    if (match(TokenType.PRINT)) return printStatement();
    if (match(TokenType.LEFT_BRACE)) return new BlockStmt(block());
    return expressionStatement();
  }

  /**
   * Build the syntax tree for the `block` production.
   * @return The `block` syntax tree
   */
  private List<Stmt> block() {
    List<Stmt> statements = new ArrayList<>();
    while (!check(TokenType.RIGHT_BRACE) && !isAtEnd()) {
      statements.add(declaration());
    }
    consume(TokenType.RIGHT_BRACE, "Expect '}' after block.");
    return statements;
  }

  /**
   * Build the syntax tree for the `print statement` production.
   * @return The `print statement` syntax tree
   */
  private Stmt printStatement() {
    final Expr value = expression();
    consume(TokenType.SEMICOLON, "Expected ';' after value.");
    return new PrintStmt(value);
  }

  /**
   * Build the syntax tree for the `expression statement` production.
   * @return The `expression statement` syntax tree
   */
  private Stmt expressionStatement() {
    final Expr expr = expression();
    consume(TokenType.SEMICOLON, "Expect ';' after expression.");
    return new ExpressionStmt(expr);
  }

  // --------------------------------------------------------------------------
  // Expression Productions
  // --------------------------------------------------------------------------

  /**
   * Build the syntax tree for the `expression` production.
   * @return The `expression` syntax tree
   */
  private Expr expression() {
    return assignment();
  }

  /**
   * Build the syntax tree for the `assignment` production.
   * @return The `assignment` syntax tree
   */
  private Expr assignment() {
    Expr expr = equality();
    
    if (match(TokenType.EQUAL)) {
      final Token equals = previous();
      // Recursively call assignment() to evaluate the right-hand side
      final Expr value = assignment();
      // Every valid assignment target is ALSO a valid normal expression
      if (expr instanceof VariableExpr) {
        final Token name = ((VariableExpr)expr).name;
        return new AssignExpr(name, value);
      }

      error(equals, "Invalid target for assignment.");
    }

    return expr;
  } 

  /**
   * Build the syntax tree for the `equality` production.
   * @return The `equality` syntax tree
   */
  private Expr equality() {
    Expr expr = comparison();
    while (match(TokenType.BANG_EQUAL, TokenType.EQUAL_EQUAL)) {
      final Token operator = previous();
      final Expr right = comparison();
      expr = new BinaryExpr(expr, operator, right);
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `comparison` production.
   * @return The `comparison` syntax tree
   */
  private Expr comparison() {
    Expr expr = term();
    while (match(TokenType.GREATER, TokenType.GREATER_EQUAL, TokenType.LESS, TokenType.LESS_EQUAL)) {
      final Token operator = previous();
      final Expr right = term();
      expr = new BinaryExpr(expr, operator, right);
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `term` production.
   * @return The `term` syntax tree
   */
  private Expr term() {
    Expr expr = factor();
    while (match(TokenType.MINUS, TokenType.PLUS)) {
      final Token operator = previous();
      final Expr right = factor();
      expr = new BinaryExpr(expr, operator, right);
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `factor` production.
   * @return The `factor` syntax tree
   */
  private Expr factor() {
    Expr expr = unary();
    while (match(TokenType.SLASH, TokenType.STAR)) {
      final Token operator = previous();
      final Expr right = unary();
      expr = new BinaryExpr(expr, operator, right);
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `unary` production.
   * @return The `unary` syntax tree
   */
  private Expr unary() {
    if (match(TokenType.BANG, TokenType.MINUS)) {
      final Token operator = previous();
      final Expr right = unary();
      return new UnaryExpr(operator, right);
    }
    return primary();
  }

  private Expr primary() {
    if (match(TokenType.FALSE)) return new LiteralExpr(false);
    if (match(TokenType.TRUE)) return new LiteralExpr(true);
    if (match(TokenType.NIL)) return new LiteralExpr(null);

    if (match(TokenType.NUMBER, TokenType.STRING)) {
      return new LiteralExpr(previous().getLiteral());
    }

    if (match(TokenType.IDENTIFER)) {
      return new VariableExpr(previous());
    }

    if (match(TokenType.LEFT_PAREN)) {
      final Expr expr = expression();
      consume(TokenType.RIGHT_PAREN, "Expect ')' after expression.");
      return new GroupingExpr(expr);
    }

    throw error(peek(), "Expected expression.");
  }

  // --------------------------------------------------------------------------
  // Utility Functions
  // --------------------------------------------------------------------------

  /**
   * Synchronize the parser state after a syntax error.
   */
  private void synchronize() {
    advance();
    while (!isAtEnd()) {
      if (previous().getType() == TokenType.SEMICOLON) return;
      switch (peek().getType()) {
        case CLASS:
        case FUN:
        case VAR:
        case FOR:
        case IF:
        case WHILE:
        case PRINT:
        case RETURN:
          return;
        default:
          break;
      }
      advance();
    }
  }

  // --------------------------------------------------------------------------
  // Utility Functions
  // --------------------------------------------------------------------------

  /**
   * Determine if the next token is of the expected type; if so, advance the
   * current token and return it, if not enter panic mode.
   * @param type The expected type
   * @param message The error message to display if the match is not found
   * @return The consumed token
   */
  private Token consume(final TokenType type, final String message) {
    if (check(type)) return advance();
    throw error(peek(), message);
  }

  /**
   * Determine if the current token is any of the provided types.
   * @param types Variadic sequence of types to check
   * @return `true` if the current token is any of `types`, `false` otherwise
   */
  private boolean match(TokenType... types) {
    for (final TokenType type : types) {
      if (check(type)) {
        advance();
        return true;
      }
    }
    return false;
  }

  /**
   * Determine if the current token is of the specified type.
   * @param type The type of interest
   * @return `true` if the current token has type `type`, `false` otherwise
   */
  private boolean check(final TokenType type) {
    if (isAtEnd()) return false;
    return peek().getType() == type;
  }

  private Token advance() {
    if (!isAtEnd()) current++;
    return previous();
  }

  /**
   * Determine if we have reached the end of the input tokens.
   * @return `true` if the current token is the EOF token, `false` otherwise
   */
  private boolean isAtEnd() {
    return peek().getType() == TokenType.EOF;
  }

  /**
   * Get the current token in the input tokens collection.
   * @return The current token
   */
  private Token peek() {
    return tokens.get(current);
  }

  /**
   * Get the previous token in the input tokens collection.
   * @return The previous token
   */
  private Token previous() {
    return tokens.get(current - 1);
  }

  /**
   * Report an error.
   * @param token The token at which the error occurred
   * @param message The error message
   * @return A new ParseError instance
   */
  private ParseError error(final Token token, final String message) {
    Lox.error(token, message);
    return new ParseError();
  }
}