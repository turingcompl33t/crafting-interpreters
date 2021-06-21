/**
 * LoxClass.java
 */

package com.craftinginterpreters.lox;

import java.util.List;
import java.util.Map;

public class LoxClass implements LoxCallable {
  /**
   * The class name.
   */
  public final String name;

  /**
   * The class methods
   */
  private final Map<String, LoxFunction> methods;

  /**
   * Construct a new LoxClass instance.
   * @param name The class name
   * @param methods The class methods
   */
  public LoxClass(final String name, final Map<String, LoxFunction> methods) {
    this.name = name;
    this.methods = methods;
  }

  /**
   * Implement the class factory function.
   * @param interpreter The associated interpreter instance
   * @param arguments The arguments to the call
   * @return The class instance (object)
   */
  @Override
  public Object call(Interpreter interpreter, final List<Object> arguments) {
    // Create the instance
    final LoxInstance instance = new LoxInstance(this);

    // If the class defines an initializer, invoke it
    final LoxFunction initializer = findMethod("init");
    if (initializer != null) {
      initializer.bind(instance).call(interpreter, arguments);
    }

    return instance;
  }

  /**
   * @return The arity of the class constructor.
   */
  @Override
  public int arity() {
    final LoxFunction initializer = findMethod("init");
    return (initializer != null) ? initializer.arity() : 0;
  }

  /**
   * Find a method defined on the class.
   * @param name The name of the method
   * @return The method, or `null` if not found
   */
  public LoxFunction findMethod(final String name) {
    if (methods.containsKey(name)) {
      return methods.get(name);
    }
    return null;
  }

  /**
   * @return A string representation of the class.
   */
  @Override
  public String toString() {
    return name;
  }
}