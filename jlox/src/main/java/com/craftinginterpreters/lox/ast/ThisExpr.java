/**
 * ThisExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The ThisExpr class represents a Lox AST `this` expression.
 */
public class ThisExpr extends Expr {
  /**
   * The `this` keyword.
   */
  public final Token keyword;

  /**
   * Construct a new ThisExpr instance.
   * @param keyword The `this` keyword
   */
  public ThisExpr(final Token keyword) {
    this.keyword = keyword;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitThisExpr(this);
  }

  @Override
  public String toString() {
    return "ThisExpr";
  }
}
