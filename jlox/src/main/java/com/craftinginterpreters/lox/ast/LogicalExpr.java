/**
 * LogicalExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The LogicalExpr class represents a Lox AST logical expression.
 */
public class LogicalExpr extends Expr {
  /**
   * The left subexpression.
   */
  public final Expr left;

  /**
   * The logical operator.
   */
  public final Token operator;

  /**
   * The right subexpression.
   */
  public final Expr right;

  /**
   * Construct a new LogicalExpr instance.
   * @param left The left subexpression
   * @param operator The logical operator
   * @param right The right subexpression
   */
  public LogicalExpr(final Expr left, final Token operator, final Expr right) {
    this.left = left;
    this.operator = operator;
    this.right = right;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitLogicalExpr(this);
  }
}