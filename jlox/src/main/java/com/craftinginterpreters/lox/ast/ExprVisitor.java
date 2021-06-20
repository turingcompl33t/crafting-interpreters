/**
 * ExprVisitor.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The ExprVisitor class implements the visitor 
 * pattern for the Lox abstract syntax tree.
 */
public interface ExprVisitor<R> {
  public R visitBinaryExpr(BinaryExpr expr);
  public R visitGroupingExpr(GroupingExpr expr);
  public R visitLiteralExpr(LiteralExpr expr);
  public R visitUnaryExpr(UnaryExpr expr);
}
