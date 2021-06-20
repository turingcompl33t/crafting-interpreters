/**
 * RuntimeError.java
 */

package com.craftinginterpreters.lox;

/**
 * The RuntimeError class represents an error
 * that occurs during the execution of a Lox program.
 */
public class RuntimeError extends RuntimeException {
  /**
   * The token at which the error occurred.
   */
  final Token token;

  RuntimeError(final Token token, final String message) {
    super(message);
    this.token = token;
  }
}
