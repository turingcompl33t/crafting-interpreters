/**
 * VariableExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The VariableExpr class represents a Lox AST variable expression.
 */
public class VariableExpr extends Expr {
  /**
   * The variable name.
   */
  public final Token name;

  /**
   * Construct a new VariableExpr instance.
   * @param name The variable name
   */
  public VariableExpr(final Token name) {
    this.name = name;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitVariableExpr(this);
  }

  @Override
  public String toString() {
    return "VariableExpr";
  }
}
