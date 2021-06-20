/**
 * IfStmt.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The IfStmt class represents a Lox AST if-statement.
 */
public class IfStmt extends Stmt {
  /**
   * The branch condition.
   */
  public final Expr condition;

  /**
   * The "then" branch.
   */
  public final Stmt thenBranch;

  /**
   * The "else" branch (optional).
   */
  public final Stmt elseBranch;

  /**
   * Construct a new IfStmt instance.
   * @param condition The branch condition
   * @param thenBranch The then branch
   * @param elseBranch The else branch (optional)
   */
  public IfStmt(final Expr condition, final Stmt thenBranch, final Stmt elseBranch) {
    this.condition = condition;
    this.thenBranch = thenBranch;
    this.elseBranch = elseBranch;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitIfStmt(this);
  }
}