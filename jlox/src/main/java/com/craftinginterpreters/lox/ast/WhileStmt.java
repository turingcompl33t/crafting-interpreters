/**
 * WhileStmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The WhileStmt class represents a Lox AST while-loop.
 */
public class WhileStmt extends Stmt {
  /**
   * The loop condition.
   */
  public final Expr condition;

  /**
   * The loop body.
   */
  public final Stmt body;

  /**
   * Construct a new WhileStmt instance.
   * @param condition The loop condition
   * @param body The loop body
   */
  public WhileStmt(final Expr condition, final Stmt body) {
    this.condition = condition;
    this.body = body;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitWhileStmt(this);
  }

  @Override
  public String toString() {
    return "WhileStmt";
  }
}