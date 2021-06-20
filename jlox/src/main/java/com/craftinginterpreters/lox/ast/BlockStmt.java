/**
 * BlockStmt.java
 */

package com.craftinginterpreters.lox.ast;

import java.util.List;

/**
 * The BlockStmt class represents a Lox AST block statement.
 */
public class BlockStmt extends Stmt {
  /**
   * The list of statements in the block.
   */
  public final List<Stmt> statements;

  /**
   * Construct a new BlockStmt instance.
   * @param statements The list of statements in the block
   */
  public BlockStmt(final List<Stmt> statements) {
    this.statements = statements;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitBlockStmt(this);
  }

  @Override
  public String toString() {
    return "BlockStmt";
  }
}
