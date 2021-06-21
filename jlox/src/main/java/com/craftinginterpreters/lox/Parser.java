/**
 * Parser.java
 */

package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Arrays;
import java.util.ArrayList;

import com.craftinginterpreters.lox.ast.*;

import jdk.nashorn.api.tree.ClassDeclarationTree;

public class Parser {
  /**
   * The ParseError class denotes an error during parsing.
   */
  private static class ParseError extends RuntimeException {}

  /**
   * The maximum number of arguments that may be passed to a Lox function call.
   */
  private static final int MAX_CALL_ARGUMENTS = 255;

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
      if (match(TokenType.CLASS)) return classDeclaration();
      if (match(TokenType.FUN)) return function("function");
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
    if (match(TokenType.IF)) return ifStatement();
    if (match(TokenType.PRINT)) return printStatement();
    if (match(TokenType.RETURN)) return returnStatement();
    if (match(TokenType.WHILE)) return whileStatement();
    if (match(TokenType.FOR)) return forStatement();
    if (match(TokenType.LEFT_BRACE)) return new BlockStmt(block());
    return expressionStatement();
  }

  /**
   * Build the syntax tree for the `whileStatement` production.
   * @return The `whileStatement` syntax tree
   */
  private Stmt whileStatement() {
    consume(TokenType.LEFT_PAREN, "Expect '(' after 'while'.");
    final Expr condition = expression();
    consume(TokenType.RIGHT_PAREN, "Expected ')' after loop condition.");
    final Stmt body = statement();
    return new WhileStmt(condition, body);
  }

  /**
   * Build the syntax tree for the `forStatement` production`.
   * @return The `forStatement` syntax tree
   */
  private Stmt forStatement() {
    consume(TokenType.LEFT_PAREN, "Expect '(' after 'for'.");

    Stmt initializer;
    if (match(TokenType.SEMICOLON)) {
      initializer = null;
    } else if (match(TokenType.VAR)) {
      initializer = varDeclaration();
    } else {
      initializer = expressionStatement();
    }

    Expr condition = !check(TokenType.SEMICOLON) ? expression() : null;
    consume(TokenType.SEMICOLON, "Expect ';' after loop condition.");

    final Expr increment = !check(TokenType.RIGHT_PAREN) ? expression() : null;
    consume(TokenType.RIGHT_PAREN, "Expect ')' after for clauses.");

    // We model a for-loop construct by de-surgaring to a while-loop
    Stmt body = statement();

    // If the increment is present, append the increment
    // as the final statement in the loop body
    if (increment != null) {
      body = new BlockStmt(
        Arrays.asList(body, new ExpressionStmt(increment)));
    }

    // If the condition is not present, model it with `while(true)`
    if (condition == null) condition = new LiteralExpr(true);
    body = new WhileStmt(condition, body);

    // Finally, if there is an initializer, wrap the entire
    // loop in a new block statement that contains the
    // initializer as the first statement in the block
    if (initializer != null) {
      body = new BlockStmt(Arrays.asList(initializer, body));
    }

    return body;
  }

  /**
   * Build the syntax tree for the `ifStatement` production.
   * @return The `ifStatement` syntax tree
   */
  private Stmt ifStatement() {
    consume(TokenType.LEFT_PAREN, "Expect '(' after 'if'.");
    final Expr condition = expression();
    consume(TokenType.RIGHT_PAREN, "Expect ')' after branch condition.");

    final Stmt thenBranch = statement();
    final Stmt elseBranch = match(TokenType.ELSE) ? statement() : null;
    return new IfStmt(condition, thenBranch, elseBranch);
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
   * Build the syntax tree for the `printStatement` production.
   * @return The `printStatement` syntax tree
   */
  private Stmt printStatement() {
    final Expr value = expression();
    consume(TokenType.SEMICOLON, "Expected ';' after value.");
    return new PrintStmt(value);
  }

  /**
   * Build the syntax tree for the `returnStatement` production.
   * @return The `returnStatement` syntax tree
   */
  private Stmt returnStatement() {
    final Token keyword = previous();
    final Expr value = !check(TokenType.SEMICOLON) ? expression() : null;
    consume(TokenType.SEMICOLON, "Expect ';' after return value.");
    return new ReturnStmt(keyword, value);
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

  /**
   * Build the syntax tree for the `classDeclaration` production.
   * @return The `classDeclaration` syntax tree
   */
  private Stmt classDeclaration() {
    final Token name = consume(TokenType.IDENTIFER, "Expect class name.");
    consume(TokenType.LEFT_BRACE, "Expect '{' before class body.");

    List<FunctionStmt> methods = new ArrayList<>();
    while (!check(TokenType.RIGHT_BRACE) && !isAtEnd()) {
      methods.add(function("method"));
    }
    consume(TokenType.RIGHT_BRACE, "Expect '}' after class body.");
    return new ClassStmt(name, methods);
  }

  /**
   * Build the syntax tree for the `functionDeclaration` production.
   * @param kind A string that identifies the kind of the function
   * @return The `functionDeclaration` syntax tree
   */
  private FunctionStmt function(final String kind) {
    final Token name = consume(TokenType.IDENTIFER, "Expect " + kind + " name.");
    consume(TokenType.LEFT_PAREN, "Expect '(' after " + kind + "name.");
    List<Token> parameters = new ArrayList<>();
    if (!check(TokenType.RIGHT_PAREN)) {
      do {
        if (parameters.size() >= MAX_CALL_ARGUMENTS) {
          error(peek(), "Maximum function call arguments exceeded.");
        }
        parameters.add(
          consume(TokenType.IDENTIFER, "Expect parameter name."));
      } while (match(TokenType.COMMA));
    }
    consume(TokenType.RIGHT_PAREN, "Expect ')' after parameters.");

    consume(TokenType.LEFT_BRACE, "Expect '{' before " + kind + " body.");
    final List<Stmt> body = block();
    
    return new FunctionStmt(name, parameters, body);
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
    Expr expr = or();
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
   * Build the syntax tree for the `or` production.
   * @return The `or` syntax tree
   */
  private Expr or() {
    Expr expr = and();
    while (match(TokenType.OR)) {
      final Token operator = previous();
      final Expr right = and();
      expr = new LogicalExpr(expr, operator, right);
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `and` production.
   * @return The `and` syntax tree
   */
  private Expr and() {
    Expr expr = equality();
    while (match(TokenType.AND)) {
      final Token operator = previous();
      final Expr right = equality();
      expr = new LogicalExpr(expr, operator, right);
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
    return call();
  }

  /**
   * Build the syntax tree for the `call` production.
   * @return The `call` syntax tree
   */
  private Expr call() {
    Expr expr = primary();
    while (true) {
      if (match(TokenType.LEFT_PAREN)) {
        expr = finishCall(expr);
      } else {
        break;
      }
    }
    return expr;
  }

  /**
   * Build the syntax tree for the `call` production.
   * @param callee The callee expression
   * @return The completed call
   */
  private Expr finishCall(final Expr callee) {
    List<Expr> arguments = new ArrayList<>();
    if (!check(TokenType.RIGHT_PAREN)) {
      do {
        if (arguments.size() >= MAX_CALL_ARGUMENTS) {
          error(peek(), "Maximum function call arguments exceeded.");
        }
        arguments.add(expression());
      } while (match(TokenType.COMMA));
    }

    final Token paren = consume(TokenType.RIGHT_PAREN, "Expect ')' after arguments.");
    return new CallExpr(callee, paren, arguments);
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