/**
 * SuperExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The SuperExpr class represents a Lox AST super expression.
 */
public class SuperExpr extends Expr {
  /**
   * The `super` keyword.
   */
  public final Token keyword;

  /**
   * The name of the method in which the expression appears.
   */
  public final Token method;

  /**
   * Construct a new SuperExpr instance.
   * @param keyword The `super` keyword
   * @param method The name of the method in which the expression appears
   */
  public SuperExpr(final Token keyword, final Token method) {
    this.keyword = keyword;
    this.method = method;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitSuperExpr(this);
  }

  @Override
  public String toString() {
    return "SuperExpr";
  }
}
