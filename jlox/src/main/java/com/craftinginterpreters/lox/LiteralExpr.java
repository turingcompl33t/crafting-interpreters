/**
 * LiteralExpr.java
 */

package com.craftinginterpreters.lox;

public class LiteralExpr extends Expr {
  /**
   * The literal value.
   */
  public final Object value;

  /**
   * Construct a new LiteralExpr.
   * @param value The literal value
   */
  LiteralExpr(final Object value) {
    this.value = value;
  }

  @Override
  <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitLiteralExpr(this);
  }
}
