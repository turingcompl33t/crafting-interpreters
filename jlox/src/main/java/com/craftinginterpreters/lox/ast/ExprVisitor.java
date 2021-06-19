/**
 * ExprVisitor.java
 */

package com.craftinginterpreters.lox;

/**
 * The ExprVisitor class implements the visitor 
 * pattern for the Lox abstract syntax tree.
 */
interface ExprVisitor<R> {
  R visitBinaryExpr(BinaryExpr expr);
  R visitGroupingExpr(GroupingExpr expr);
  R visitLiteralExpr(LiteralExpr expr);
  R visitUnaryExpr(UnaryExpr expr);
}