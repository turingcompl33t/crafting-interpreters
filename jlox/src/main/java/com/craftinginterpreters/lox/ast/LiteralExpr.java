/**
 * LiteralExpr.java
 */

package com.craftinginterpreters.lox.ast;

public class LiteralExpr extends Expr {
  /**
   * The literal value.
   */
  public final Object value;

  /**
   * Construct a new LiteralExpr.
   * @param value The literal value
   */
  public LiteralExpr(final Object value) {
    this.value = value;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitLiteralExpr(this);
  }

  @Override
  public String toString() {
    return "LiteralExpr";
  }
}
