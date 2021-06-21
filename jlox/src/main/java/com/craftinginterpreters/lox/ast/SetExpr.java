/**
 * SetExpr.java
 */

package com.craftinginterpreters.lox.ast;

import com.craftinginterpreters.lox.Token;

/**
 * The SetExpr class represents a Lox AST set expression.
 */
public class SetExpr extends Expr {
  /**
   * The instance on which the set() is performed.
   */
  public final Expr object;

  /**
   * The name of the set field.
   */
  public final Token name;

  /**
   * The value to which the field is set.
   */
  public final Expr value;

  /**
   * Construct a new SetExpr instance.
   * @param object The object on which the set() is performed
   * @param name The name of the set field
   * @param value The value to which the field is set
   */
  public SetExpr(final Expr object, final Token name, final Expr value) {
    this.object = object;
    this.name = name;
    this.value = value;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitSetExpr(this);
  }

  @Override
  public String toString() {
    return "SetExpr";
  }
}
