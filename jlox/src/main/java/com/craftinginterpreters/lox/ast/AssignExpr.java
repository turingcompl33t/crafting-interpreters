/**
 * AssignExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The AssignExpr class represents a Lox AST assignment expression.
 */
public class AssignExpr extends Expr {
  /**
   * The identifier for the variable to which we assign.
   */
  public final Token name;

  /**
   * The assigned value.
   */
  public final Expr value;

  /**
   * Construct a new AssignExpr instance.
   * @param name The variable identifier
   * @param value The assigned value
   */
  public AssignExpr(final Token name, final Expr value) {
    this.name = name;
    this.value = value;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitAssignExpr(this);
  }

  @Override
  public String toString() {
    return "AssignExpr";
  }
}