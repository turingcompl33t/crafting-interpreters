/**
 * GroupingExpr.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The GroupingExpr class implements grouping expressions.
 */
public class GroupingExpr extends Expr {
  /**
   * The grouped expression.
   */
  public final Expr expression;
  
  /**
   * Construct a new GroupingExpr.
   * @param expression The grouped expression
   */
  public GroupingExpr(Expr expression) {
    this.expression = expression;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitGroupingExpr(this);
  }
}