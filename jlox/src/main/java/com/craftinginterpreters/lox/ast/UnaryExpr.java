/**
 * UnaryExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The UnaryExpr class implements unary expressions.
 */
public class UnaryExpr extends Expr {
  /**
   * The unary operator.
   */
  public Token operator;

  /**
   * The expression.
   */
  public Expr expression;

  /**
   * Construct a new UnaryExpr.
   * @param operator The unary operator
   * @param expression The expression
   */
  public UnaryExpr(final Token operator, final Expr expression) {
    this.operator = operator;
    this.expression = expression;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitUnaryExpr(this);
  }
}
