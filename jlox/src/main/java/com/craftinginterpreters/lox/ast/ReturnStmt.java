/**
 * ReturnStmt.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The ReturnStmt class represents a Lox AST return statement.
 */
public class ReturnStmt extends Stmt {
  /**
   * The "return" keyword.
   */
  public final Token keyword;

  /**
   * The returned value.
   */
  public final Expr value;

  /**
   * Construct a new ReturnStmt instance.
   * @param keyword The "return" keyword
   * @param value The returned value
   */
  public ReturnStmt(final Token keyword, final Expr value) {
    this.keyword = keyword;
    this.value = value;
  }

  @Override
  public <R> R accept(StmtVisitor<R> visitor) {
    return visitor.visitReturnStmt(this);
  }

  @Override
  public String toString() {
    return "ReturnStmt";
  }
}
