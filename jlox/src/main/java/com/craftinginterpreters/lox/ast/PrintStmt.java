/**
 * PrintStmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The PrintStmt class represents a Lox AST print statement.
 */
public class PrintStmt extends Stmt {
  /**
   * The expression to be printed.
   */
  public final Expr expression;

  /**
   * Construct a new PrintStmt expression.
   * @param expression
   */
  public PrintStmt(final Expr expression) {
    this.expression = expression;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitPrintStmt(this);
  }

  @Override
  public String toString() {
    return "PrintStmt";
  }
}