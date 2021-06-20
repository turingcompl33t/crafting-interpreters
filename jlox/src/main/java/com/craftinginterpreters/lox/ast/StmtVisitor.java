/**
 * StmtVisitor.java
 */

package com.craftinginterpreters.lox.ast;

/**
 * The StmtVisitor class implements the visitor pattern for Lox AST statements.
 */
public interface StmtVisitor<R> {
  R visitExpressionStmt(ExpressionStmt stmt);
  R visitPrintStmt(PrintStmt stmt);
}
