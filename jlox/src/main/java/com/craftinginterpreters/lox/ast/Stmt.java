/**
 * Stmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The Stmt class is the base class for all AST statement types.
 */
public abstract class Stmt {
  /**
   * Visitor implementation for Lox AST.
   * @param <R> Visitor return type
   * @param visitor The visitor instance
   * @return Visitor-dependent value
   */
  public abstract <R> R accept(StmtVisitor<R> visitor);

  /**
   * Create a string representation of the statement.
   * @return The string representation
   */
  public abstract String toString();
}
