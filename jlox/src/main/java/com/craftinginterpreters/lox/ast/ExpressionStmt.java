/**
 * ExpressionStmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The ExpressionStatement class represents a Lox AST expression statement.
 */
public class ExpressionStmt extends Stmt {
  /**
   * The expression that composes the statement.
   */
  public final Expr expression;

  /**
   * Construct a new ExpressionStmt instance.
   * @param expression
   */
  public ExpressionStmt(final Expr expression) {
    this.expression = expression;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitExpressionStmt(this);
  }
}
