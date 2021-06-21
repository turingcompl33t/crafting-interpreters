/**
 * Environment.java
 */

package com.craftinginterpreters.lox;

import java.util.Map;
import java.util.HashMap;

/**
 * The Environment class implements a lexical scope;
 * it maps variable identifiers to runtime values.
 */
public class Environment {
  /**
   * The underlying map.
   */
  private final Map<String, Object> values = new HashMap<>();

  /**
   * A reference to the enclosing environment.
   */
  public final Environment enclosing;

  /**
   * Construct a new environment instance (root-scope).
   */
  public Environment() {
    this.enclosing = null;
  }

  /**
   * Construct a new Environment instance (non-root scope).
   * @param enclosing The enclosing environment
   */
  public Environment(final Environment enclosing) {
    this.enclosing = enclosing;
  }

  /**
   * Get the runtime value for a variable identified by `name`.
   * @param name The variable identifier
   * @return The runtime value for the variable
   * @throws RuntimeError If the variable is not defined
   */
  Object get(final Token name) throws RuntimeError {
    final String lexeme = name.getLexeme();
    if (values.containsKey(lexeme)) {
      return values.get(lexeme);
    }
    
    // Traverse to the enclosing scope and check there
    if (enclosing != null) return enclosing.get(name);

    throw new RuntimeError(name, "Undefined variable '" + lexeme + "'");
  }

  /**
   * Get the runtime value for a variable at the specified distance.
   * @param distance The distance at which to search for the variable
   * @param name The variable identifier
   * @return The runtime value for the variable
   * @throws RuntimeError If the variable is not defined
   */
  public Object getAt(final int distance, final String name) throws RuntimeError {
    return ancestor(distance).values.get(name);
  }

  /**
   * Define a variable in the environment.
   * @param name The variable identifier
   * @param value The variable value
   */
  public void define(final String name, final Object value) {
    // NOTE: We allow redefinition of variables; if we
    // did not, we could raise a RuntimeError here
    values.put(name, value);
  }

  /**
   * Assign a variable.
   * @param name The variable identifier
   * @param value The runtime value
   */
  public void assign(final Token name, final Object value) {
    final String lexeme = name.getLexeme();
    if (values.containsKey(lexeme)) {
      values.put(lexeme, value);
      return;
    }

    // Traverse to the enclosing scope and check there
    if (enclosing != null) {
      enclosing.assign(name, value);
      return;
    }

    throw new RuntimeError(name, "Undefined variable '" + lexeme + "'");
  }

  /**
   * Assign a variable at the given distance.
   * @param distance The distance at which to perform the assignment
   * @param name The variable identifier
   * @param value The runtime value
   */
  public void assignAt(final int distance, final Token name, final Object value) {
    ancestor(distance).values.put(name.getLexeme(), value);
  }

  /**
   * Get the ancestor environment at the specified distance.
   * @param distance The distance
   * @return The ancestor environment
   */
  private Environment ancestor(final int distance) {
    Environment env = this;
    for (int i = 0; i < distance; ++i) {
      env = env.enclosing;
    }
    return env;
  }
}
