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
  private final Environment enclosing;

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
   * Define a variable in the environment.
   * @param name The variable identifier
   * @param value The variable value
   */
  public void define(final String name, final Object value) {
    // NOTE: We allow redefinition of variables; if we
    // did not, we could raise a RuntimeError here
    values.put(name, value);
  }

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
}
