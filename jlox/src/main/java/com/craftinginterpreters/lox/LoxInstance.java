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
   * Construct a new LoxInstance instance.
   * @param klass The associated class
   */
  public LoxInstance(final LoxClass klass) {
    this.klass = klass;
  }

  @Override
  public String toString() {
    return klass.name + " instance";
  }
}