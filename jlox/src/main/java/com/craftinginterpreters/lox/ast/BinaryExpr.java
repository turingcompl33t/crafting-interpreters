/**
 * BinaryExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The BinaryExpr class implements binary expressions.
 */
public class BinaryExpr extends Expr {
  /**
   * The left subexpression.
   */
  public final Expr left;

  /**
   * The right subexpression.
   */
  public final Expr right;

  /**
   * The binary operator.
   */
  public final Token operator;

  /**
   * Construct a new BinaryExpr.
   * @param left The left subexpression
   * @param operator The binary operator
   * @param right The right subexpression
   */
  public BinaryExpr(final Expr left, final Token operator, final Expr right) {
    this.left = left;
    this.operator = operator;
    this.right = right;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitBinaryExpr(this);
  }
}
