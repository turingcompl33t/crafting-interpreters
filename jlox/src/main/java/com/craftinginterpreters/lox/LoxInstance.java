/**
 * LoxInstance.java
 */

package com.craftinginterpreters.lox;

import java.util.HashMap;
import java.util.Map;

/**
 * The LoxInstance class represents a runtime class instantiation.
 */
public class LoxInstance {
  /** 
   * The associated class.
   */
  private final LoxClass klass;

  /**
   * The state for instance properties.
   */
  private final Map<String, Object> fields = new HashMap<>();

  /**
   * Construct a new LoxInstance instance.
   * @param klass The associated class
   */
  public LoxInstance(final LoxClass klass) {
    this.klass = klass;
  }

  /**
   * Get the value of an instance property.
   * @param name The name of the property
   * @return The value of the field
   * @throws RuntimeError On invalid property access
   */
  public Object get(final Token name) {
    // NOTE: The subtle difference between properties and fields

    // The get expression might return the value of a field
    if (fields.containsKey(name.getLexeme())) {
      return fields.get(name.getLexeme());
    }

    // OR the get expression might return a method defined in the class
    final LoxFunction method = klass.findMethod(name.getLexeme());
    if (method != null) return method;

    throw new RuntimeError(name,
      "Undefined property '" + name.getLexeme() + "'");
  }

  /**
   * Set the value of the field on the instance.
   * @param name The name of the field
   * @param value The value to which the field is set
   */
  public void set(final Token name, final Object value) {
    fields.put(name.getLexeme(), value);
  }

  @Override
  public String toString() {
    return klass.name + " instance";
  }
}