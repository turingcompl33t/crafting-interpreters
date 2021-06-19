/**
 * AstPrinter.java
 */

package com.craftinginterpreters.lox;

/**
 * The AstPrinter class implements a simple AST printer
 * that displays explicit nesting of the Lox AST.
 */
public class AstPrinter implements ExprVisitor<String> {
  /**
   * Print the AST rooted at `expr`.
   * @param expr The expression from which to begin AST printing
   * @return A string representation of the AST, with explicit nesting
   */
  String print(Expr expr) {
    return expr.accept(this);
  }

  @Override
  public String visitBinaryExpr(final BinaryExpr expr) {
    return parenthesize(expr.operator.lexeme, expr.left, expr.right);
  }

  @Override
  public String visitGroupingExpr(final GroupingExpr expr) {
    return parenthesize("group", expr.expression);
  }

  @Override
  public String visitLiteralExpr(final LiteralExpr expr) {
    if (expr.value == null) return "nil";
    return expr.value.toString();
  }

  @Override
  public String visitUnaryExpr(final UnaryExpr expr) {
    return parenthesize(expr.operator.lexeme, expr.expression);
  }

  /**
   * Wrap an expression tree in a set of parenthesis.
   * @param name The identifier for the expression
   * @param exprs A variable number of subexpressions
   * @return The string representation
   */
  private String parenthesize(final String name, Expr... exprs) {
    StringBuilder builder = new StringBuilder();
    builder.append("(").append(name);
    for (final Expr expr : exprs) {
      builder.append(" ");
      builder.append(expr.accept(this));
    }
    builder.append(")");
    return builder.toString();
  }
}
