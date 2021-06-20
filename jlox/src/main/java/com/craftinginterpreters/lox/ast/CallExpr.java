/**
 * CallExpr.java
 */

package com.craftinginterpreters.lox.ast;

import java.util.List;
import com.craftinginterpreters.lox.Token;

/**
 * The CallExpr class represents a Lox AST function call expression.
 */
public class CallExpr extends Expr {
  /**
   * The callee expression.
   */
  public final Expr callee;

  /**
   * The parenthesis that ends the call().
   */
  public final Token paren;

  /**
   * The arguments to the call.
   */
  public final List<Expr> arguments;

  /**
   * Construct a new CallExpr instance.
   * @param callee The expression for the callee
   * @param paren The parenthesis that ends the call()
   * @param arguments The list of arguments to the call
   */
  public CallExpr(final Expr callee, final Token paren, final List<Expr> arguments) {
    this.callee = callee;
    this.paren = paren;
    this.arguments = arguments;
  }

  @Override
  public <R> R accept(ExprVisitor<R> visitor) {
    return visitor.visitCallExpr(this);
  }

  @Override
  public String toString() {
    return "CallExpr";
  }
}