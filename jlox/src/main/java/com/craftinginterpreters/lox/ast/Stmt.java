/**
 * Stmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The Stmt class is the base class for all AST statement types.
 */
public abstract class Stmt {
  public abstract <R> R accept(StmtVisitor<R> visitor);
}
