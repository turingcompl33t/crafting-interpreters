/**
 * Expr.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The base class for all AST expressions.
 */
public abstract class Expr {
  /**
   * Visitor implementation for Lox AST.
   * @param <R> Visitor return type
   * @param visitor The visitor instance
   * @return Visitor-dependent value
   */
  public abstract <R> R accept(ExprVisitor<R> visitor);

  /**
   * Create a string representation for the expression.
   * @return The string representation
   */
  public abstract String toString();
}
