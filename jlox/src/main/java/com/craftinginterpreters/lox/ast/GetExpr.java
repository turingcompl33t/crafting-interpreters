/**
 * GetExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The GetExpr class represents a Lox AST get expression.
 */
public class GetExpr extends Expr {
  /**
   * The object on which the get() is performed.
   */
  public final Expr object;

  /**
   * The name of the property.
   */
  public final Token name;

  /**
   * Construct a new GetExpr instance.
   * @param object The object on which the get() is performed
   * @param name The name of the property
   */
  public GetExpr(final Expr object, final Token name) {
    this.object = object;
    this.name = name;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitGetExpr(this);
  }

  @Override
  public String toString() {
    return "GetExpr";
  }
}
