/**
 * LoxCallable.java
 */

package com.craftinginterpreters.lox;

import java.util.List;

/**
 * The LoxCallable interface defines the interface
 * for all "things that can be called like functions".
 */
public interface LoxCallable {
  /**
   * Execute the callable on the provided interpreter.
   * @param interpreter The interpreter instance
   * @param arguments The arguments to the call
   * @return The return value from the call
   */
  public Object call(Interpreter interpreter, final List<Object> arguments);

  /**
   * Return the arity of the callable.
   * @return The callable's arity
   */
  public int arity();
}
