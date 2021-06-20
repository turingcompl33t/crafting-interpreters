/**
 * Return.java
 */

package com.craftinginterpreters.lox;

/**
 * The Return class implements exception-like semantics
 * for managing the control flow during the interpretation
 * of `return` statements within Lox.
 */
public class Return extends RuntimeException {
  /**
   * The returned value.
   */
  public final Object value;

  /**
   * Construct a new Return instance.
   * @param value The return value
   */
  public Return(final Object value) {
    super(null, null, false, false);
    this.value = value;
  }
}
