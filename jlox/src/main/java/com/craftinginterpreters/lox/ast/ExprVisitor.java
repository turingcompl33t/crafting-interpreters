/**
 * ExprVisitor.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The ExprVisitor class implements the visitor 
 * pattern for the Lox abstract syntax tree.
 */
public interface ExprVisitor<R> {
  public R visitAssignExpr(AssignExpr expr);
  public R visitBinaryExpr(BinaryExpr expr);
  public R visitCallExpr(CallExpr expr);
  public R visitGetExpr(GetExpr expr);
  public R visitGroupingExpr(GroupingExpr expr);
  public R visitLiteralExpr(LiteralExpr expr);
  public R visitLogicalExpr(LogicalExpr expr);
  public R visitSetExpr(SetExpr expr);
  public R visitSuperExpr(SuperExpr expr);
  public R visitUnaryExpr(UnaryExpr expr);
  public R visitVariableExpr(VariableExpr expr);
  public R visitThisExpr(ThisExpr expr);
}
