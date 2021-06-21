/**
 * LoxClass.java
 */

package com.craftinginterpreters.lox;

import java.util.List;

public class LoxClass implements LoxCallable {
  /**
   * The class name.
   */
  public final String name;

  /**
   * Construct a new LoxClass instance.
   * @param name The class name
   */
  public LoxClass(final String name) {
    this.name = name;
  }

  /**
   * Implement the class factory function.
   * @param interpreter The associated interpreter instance
   * @param arguments The arguments to the call
   * @return The class instance (object)
   */
  @Override
  public Object call(Interpreter interpreter, final List<Object> arguments) {
    return new LoxInstance(this);
  }

  /**
   * @return The arity of the class constructor.
   */
  @Override
  public int arity() {
    return 0;
  }

  /**
   * @return A string representation of the class.
   */
  @Override
  public String toString() {
    return name;
  }
}