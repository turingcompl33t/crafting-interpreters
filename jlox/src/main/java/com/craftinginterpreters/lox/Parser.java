/**
 * Parser.java
 */

package com.craftinginterpreters.lox;

import java.util.List;

import com.craftinginterpreters.lox.ast.Expr;
import com.craftinginterpreters.lox.ast.BinaryExpr;
import com.craftinginterpreters.lox.ast.GroupingExpr;
import com.craftinginterpreters.lox.ast.LiteralExpr;
import com.craftinginterpreters.lox.ast.UnaryExpr;

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
   * @return The root of the AST
   */
  public Expr parse() {
    try {
      return expression();
    } catch (ParseError e) {
      return null;
    }
  }

  // --------------------------------------------------------------------------
  // Productions
  // --------------------------------------------------------------------------

  /**
   * Build the syntax tree for the `expression` production.
   * @return The `expression` syntax tree
   */
  private Expr expression() {
    return equality();
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