/**
 * Expr.java
 */

package com.craftinginterpreters.lox;

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
  abstract <R> R accept(ExprVisitor<R> visitor);
}